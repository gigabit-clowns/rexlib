// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/stream_timeline.hpp>

#include "mock/mock_event_recorder.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>

using namespace rexlib;

namespace
{

/// Make a handle standing for a stream, without creating one. A timeline
/// only ever passes its stream on to the recorder.
cudaStream_t make_test_stream(unsigned index) noexcept
{
	const auto value = static_cast<std::uintptr_t>(index + 1) << 12;
	return reinterpret_cast<cudaStream_t>(value);
}

/**
 * @brief A timeline over a mocked recorder, with three commands recorded.
 *
 * The commands have the ids 0, 1 and 2, and their points the tickets 11, 12
 * and 13. A command is still running unless a test says otherwise. Giving a
 * point back is allowed throughout, since the timeline gives back what it
 * still holds on its way out. A test that is about either call says so
 * itself, which takes precedence.
 */
class timeline_fixture
{
public:
	timeline_fixture()
		: m_recorder(std::make_shared<cuda::mock_event_recorder>())
		, m_stream(make_test_stream(0))
		, m_release_expectation(
			NAMED_ALLOW_CALL(*m_recorder, release(trompeloeil::_))
		)
		, m_is_complete_expectation(
			NAMED_ALLOW_CALL(*m_recorder, is_complete(trompeloeil::_))
				.RETURN(false)
		)
		, m_timeline(m_recorder)
	{
		for (const cuda::event_recorder::ticket point : {11, 12, 13})
		{
			REQUIRE_CALL(*m_recorder, record(m_stream))
				.RETURN(point);

			m_timeline.record(m_stream);
		}
	}

	cuda::stream_timeline& get() noexcept
	{
		return m_timeline;
	}

	cuda::mock_event_recorder& get_recorder() noexcept
	{
		return *m_recorder;
	}

	cudaStream_t get_stream() const noexcept
	{
		return m_stream;
	}

private:
	std::shared_ptr<cuda::mock_event_recorder> m_recorder;
	cudaStream_t m_stream;

	// Declared before the timeline, so that they are destroyed after it.
	std::unique_ptr<trompeloeil::expectation> m_release_expectation;
	std::unique_ptr<trompeloeil::expectation> m_is_complete_expectation;

	cuda::stream_timeline m_timeline;
};

} // anonymous namespace

TEST_CASE(
	"stream_timeline should refuse to be built without a recorder",
	"[stream_timeline]"
)
{
	CHECK_THROWS_AS( cuda::stream_timeline(nullptr), std::invalid_argument );
}

TEST_CASE(
	"stream_timeline should give every command the next id and a point of "
	"its stream",
	"[stream_timeline]"
)
{
	const auto recorder = std::make_shared<cuda::mock_event_recorder>();
	const auto stream = make_test_stream(0);
	ALLOW_CALL(*recorder, release(trompeloeil::_));
	ALLOW_CALL(*recorder, is_complete(trompeloeil::_))
		.RETURN(false);

	cuda::stream_timeline timeline(recorder);

	trompeloeil::sequence sequence;
	REQUIRE_CALL(*recorder, record(stream))
		.RETURN(11)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(*recorder, record(stream))
		.RETURN(12)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(*recorder, record(stream))
		.RETURN(13)
		.IN_SEQUENCE(sequence);

	const auto first = timeline.record(stream);
	CHECK( timeline.record(stream) == first + 1 );
	CHECK( timeline.record(stream) == first + 2 );
}

TEST_CASE(
	"stream_timeline should answer for a command by asking about its point",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	const auto &timeline = fixture.get();

	SECTION( "that it is still running" )
	{
		REQUIRE_CALL(recorder, is_complete(12u))
			.RETURN(false);

		CHECK_FALSE( timeline.is_complete(1) );
	}

	SECTION( "that it has finished" )
	{
		REQUIRE_CALL(recorder, is_complete(12u))
			.RETURN(true);

		CHECK( timeline.is_complete(1) );
	}
}

TEST_CASE(
	"stream_timeline should wait for a command through its point",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;

	REQUIRE_CALL(fixture.get_recorder(), wait(12u));

	fixture.get().wait(1);
}

TEST_CASE(
	"stream_timeline should make a stream wait for a command through its "
	"point",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	const auto waiter = make_test_stream(1);

	REQUIRE_CALL(fixture.get_recorder(), enqueue_wait(waiter, 12u));

	fixture.get().enqueue_wait(waiter, 1);
}

TEST_CASE(
	"recording on a stream_timeline should give back the points that have "
	"been reached",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &timeline = fixture.get();

	// The stream runs in order, so the first point that has not been reached
	// ends the search: nothing after it is asked about.
	trompeloeil::sequence sequence;
	REQUIRE_CALL(recorder, is_complete(11u))
		.RETURN(true)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, release(11u))
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, is_complete(12u))
		.RETURN(true)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, release(12u))
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, is_complete(13u))
		.RETURN(false)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, record(fixture.get_stream()))
		.RETURN(14)
		.IN_SEQUENCE(sequence);

	CHECK( timeline.record(fixture.get_stream()) == 3 );

	// Their commands are known to have finished from then on, without the
	// recorder having to be asked. The tickets may stand for something else
	// by now.
	FORBID_CALL(recorder, is_complete(11u));
	FORBID_CALL(recorder, is_complete(12u));
	FORBID_CALL(recorder, wait(trompeloeil::_));
	FORBID_CALL(recorder, enqueue_wait(trompeloeil::_, trompeloeil::_));

	CHECK( timeline.is_complete(0) );
	CHECK( timeline.is_complete(1) );
	timeline.wait(0);
	timeline.wait(1);
	timeline.enqueue_wait(make_test_stream(1), 0);
	timeline.enqueue_wait(make_test_stream(1), 1);
}

TEST_CASE(
	"a stream_timeline should keep answering for the commands that are still "
	"running after giving points back",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &timeline = fixture.get();

	{
		ALLOW_CALL(recorder, is_complete(11u))
			.RETURN(true);
		REQUIRE_CALL(recorder, record(fixture.get_stream()))
			.RETURN(14);

		timeline.record(fixture.get_stream());
	}

	// The ids did not move with the points that were given back.
	REQUIRE_CALL(recorder, is_complete(12u))
		.RETURN(false);
	REQUIRE_CALL(recorder, wait(13u));
	REQUIRE_CALL(recorder, is_complete(14u))
		.RETURN(true);

	CHECK_FALSE( timeline.is_complete(1) );
	timeline.wait(2);
	CHECK( timeline.is_complete(3) );
}

TEST_CASE(
	"a stream_timeline should keep its points while a thread waits for one",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &timeline = fixture.get();
	const auto stream = fixture.get_stream();

	{
		// A command is submitted while the wait is going on. Every point
		// has been reached, but giving one back would let it be captured
		// again under the thread that is waiting for it.
		FORBID_CALL(recorder, release(trompeloeil::_));
		ALLOW_CALL(recorder, is_complete(trompeloeil::_))
			.RETURN(true);
		REQUIRE_CALL(recorder, record(stream))
			.RETURN(14);
		REQUIRE_CALL(recorder, wait(12u))
			.LR_SIDE_EFFECT(timeline.record(stream));

		timeline.wait(1);
	}

	// They go back with the next command, now that nobody is waiting.
	ALLOW_CALL(recorder, is_complete(trompeloeil::_))
		.RETURN(true);
	REQUIRE_CALL(recorder, release(11u));
	REQUIRE_CALL(recorder, release(12u));
	REQUIRE_CALL(recorder, release(13u));
	REQUIRE_CALL(recorder, release(14u));
	REQUIRE_CALL(recorder, record(stream))
		.RETURN(15);

	timeline.record(stream);
}

TEST_CASE(
	"a stream_timeline should give points back again after a wait that "
	"failed",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &timeline = fixture.get();

	{
		REQUIRE_CALL(recorder, wait(12u))
			.THROW(std::runtime_error("no answer"));

		CHECK_THROWS_AS( timeline.wait(1), std::runtime_error );
	}

	ALLOW_CALL(recorder, is_complete(11u))
		.RETURN(true);
	REQUIRE_CALL(recorder, release(11u));
	REQUIRE_CALL(recorder, record(fixture.get_stream()))
		.RETURN(14);

	timeline.record(fixture.get_stream());
}

TEST_CASE(
	"a stream_timeline should record nothing when an earlier point cannot be "
	"queried",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &timeline = fixture.get();

	{
		REQUIRE_CALL(recorder, is_complete(11u))
			.THROW(std::runtime_error("no answer"));
		FORBID_CALL(recorder, record(trompeloeil::_));
		FORBID_CALL(recorder, release(trompeloeil::_));

		CHECK_THROWS_AS(
			timeline.record(fixture.get_stream()),
			std::runtime_error
		);
	}

	// The next command takes the id the failed one would have had.
	REQUIRE_CALL(recorder, record(fixture.get_stream()))
		.RETURN(14);

	CHECK( timeline.record(fixture.get_stream()) == 3 );
}

TEST_CASE(
	"stream_timeline should refuse an id it never gave",
	"[stream_timeline]"
)
{
	timeline_fixture fixture;
	const auto &timeline = fixture.get();

	FORBID_CALL(fixture.get_recorder(), wait(trompeloeil::_));
	FORBID_CALL(
		fixture.get_recorder(),
		enqueue_wait(trompeloeil::_, trompeloeil::_)
	);

	CHECK_THROWS_AS( timeline.is_complete(3), std::out_of_range );
	CHECK_THROWS_AS( timeline.wait(3), std::out_of_range );
	CHECK_THROWS_AS(
		timeline.enqueue_wait(make_test_stream(1), 3),
		std::out_of_range
	);
}

TEST_CASE(
	"a stream_timeline should give back the points it still holds when it "
	"dies",
	"[stream_timeline]"
)
{
	const auto recorder = std::make_shared<cuda::mock_event_recorder>();
	const auto stream = make_test_stream(0);

	REQUIRE_CALL(*recorder, release(11u));
	REQUIRE_CALL(*recorder, release(12u));

	{
		cuda::stream_timeline timeline(recorder);

		ALLOW_CALL(*recorder, is_complete(trompeloeil::_))
			.RETURN(false);
		REQUIRE_CALL(*recorder, record(stream))
			.RETURN(11);
		REQUIRE_CALL(*recorder, record(stream))
			.RETURN(12);

		timeline.record(stream);
		timeline.record(stream);
	}
}
