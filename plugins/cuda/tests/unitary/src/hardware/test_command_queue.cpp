// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/command_queue.hpp>

#include <hardware/pooled_event_recorder.hpp>
#include <hardware/stream_timeline.hpp>

#include <hardware/caching_allocator/buffer.hpp>

#include "caching_allocator/mock/mock_buffer.hpp"
#include "caching_allocator/test_caching_allocator.hpp"
#include "mock/mock_command_queue.hpp"
#include "mock/mock_command_timeline.hpp"
#include "mock/mock_stream_program.hpp"
#include "test_driver.hpp"
#include "test_stream_blocker.hpp"
#include "test_stream_probe.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/hardware/program.hpp>

#include <memory>
#include <stdexcept>

using namespace rexlib;

namespace
{

/**
 * @brief A queue of the first device, and buffers of this backend for its
 * commands.
 *
 * The buffers are cut out of ordinary host memory, since a queue only has to
 * tell whose they are. The allocator behind them is declared first, so that
 * it dies last: it waits on its way out for the commands that used them.
 */
class queue_fixture
{
public:
	queue_fixture()
		: m_queue(0, std::make_shared<cuda::pooled_event_recorder>(0))
	{
	}

	cuda::command_queue& get() noexcept
	{
		return m_queue;
	}

	std::shared_ptr<rexlib::buffer> allocate()
	{
		command_token pending;
		return m_buffers.get().allocate(
			1024,
			REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES,
			nullptr,
			pending
		);
	}

private:
	cuda::caching_allocator_fixture m_buffers;
	cuda::command_queue m_queue;
};

} // anonymous namespace

TEST_CASE(
	"a command_queue should refuse to be built without an event recorder",
	"[command_queue]"
)
{
	CHECK_THROWS_AS(
		cuda::command_queue(0, nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"a command_queue should own a stream on its device",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto recorder = std::make_shared<cuda::pooled_event_recorder>(0);
	const cuda::command_queue first(0, recorder);
	const cuda::command_queue second(0, recorder);

	CHECK( first.get_ordinal() == 0 );
	CHECK( first.get_handle() != nullptr );
	CHECK( first.get_handle() != second.get_handle() );
}

TEST_CASE(
	"a command_queue should have a timeline of its own",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto recorder = std::make_shared<cuda::pooled_event_recorder>(0);
	const cuda::command_queue first(0, recorder);
	const cuda::command_queue second(0, recorder);

	REQUIRE( first.get_timeline() != nullptr );
	CHECK( first.get_timeline() != second.get_timeline() );
}

TEST_CASE(
	"the timeline of a command_queue should follow its stream",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto recorder = std::make_shared<cuda::pooled_event_recorder>(0);
	const cuda::command_queue queue(0, recorder);
	const auto &timeline = queue.get_timeline();

	cuda::stream_blocker blocker(queue.get_handle());
	const command_token token(
		timeline,
		timeline->record(queue.get_handle())
	);

	CHECK_FALSE( token.is_complete() );

	blocker.release();
	token.wait();

	CHECK( token.is_complete() );
}

TEST_CASE(
	"the timeline of a command_queue should outlive it",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	command_token token;
	{
		const cuda::command_queue queue(
			0,
			std::make_shared<cuda::pooled_event_recorder>(0)
		);
		const auto &timeline = queue.get_timeline();
		token = command_token(
			timeline,
			timeline->record(queue.get_handle())
		);
	}

	// An array may hold the token of a command long after the queue that
	// ran it is gone.
	CHECK_NOTHROW( token.wait() );
	CHECK( token.is_complete() );
}

TEST_CASE(
	"synchronizing a command_queue should wait for its stream",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::command_queue queue(
		0,
		std::make_shared<cuda::pooled_event_recorder>(0)
	);
	const auto &timeline = queue.get_timeline();
	const command_token token(
		timeline,
		timeline->record(queue.get_handle())
	);

	queue.synchronize();

	CHECK( token.is_complete() );
}

TEST_CASE(
	"a command_queue should refuse a command without a program",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	cuda::command_queue queue(
		0,
		std::make_shared<cuda::pooled_event_recorder>(0)
	);

	CHECK_THROWS_AS( queue.submit(command()), std::invalid_argument );
}

TEST_CASE(
	"a command_queue should refuse a program this backend did not make",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;

	CHECK_THROWS_AS(
		fixture.get().submit(command(std::make_shared<rexlib::program>())),
		std::invalid_argument
	);
}

TEST_CASE(
	"a command_queue should launch the program of a command on its stream, "
	"with its buffers",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	const auto program = std::make_shared<cuda::mock_stream_program>();
	const auto output = fixture.allocate();
	const auto input = fixture.allocate();
	const auto scratch = fixture.allocate();

	REQUIRE_CALL(
		*program,
		launch(
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_,
			fixture.get().get_handle()
		)
	)
		.LR_WITH(_1.size() == 1 && _1[0] == output)
		.LR_WITH(_2.size() == 1 && _2[0] == input)
		.LR_WITH(_3.size() == 1 && _3[0] == scratch);

	command cmd(program);
	cmd.bind_outputs({output}).bind_inputs({input}).bind_scratch({scratch});
	fixture.get().submit(std::move(cmd));
}

TEST_CASE(
	"a command_queue should return the token of the work it issued",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	auto &queue = fixture.get();
	const auto program = std::make_shared<cuda::mock_stream_program>();
	ALLOW_CALL(
		*program,
		launch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
	);

	cuda::stream_blocker blocker(queue.get_handle());

	const auto token = queue.submit(command(program));

	CHECK( token.get_timeline() == queue.get_timeline() );
	CHECK_FALSE( token.is_complete() );

	blocker.release();
	token.wait();

	CHECK( token.is_complete() );
}

TEST_CASE(
	"a command_queue should tell the buffers of a command which command uses "
	"them",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	auto &queue = fixture.get();
	const auto program = std::make_shared<cuda::mock_stream_program>();
	ALLOW_CALL(
		*program,
		launch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
	);

	const auto output = fixture.allocate();
	const auto input = fixture.allocate();
	const auto scratch = fixture.allocate();

	// Held busy, so that the command is still running when its buffers are
	// asked.
	cuda::stream_blocker blocker(queue.get_handle());

	command cmd(program);
	cmd.bind_outputs({output}).bind_inputs({input}).bind_scratch({scratch});
	const auto token = queue.submit(std::move(cmd));

	// A buffer may die before the command does, and this is how the memory
	// finds out what it still has to wait for.
	for (const auto &operand : {output, input, scratch})
	{
		const auto uses = cuda::buffer::cast(*operand).get_recorded_uses();
		REQUIRE( uses.size() == 1 );
		CHECK( uses[0].get_timeline() == queue.get_timeline() );
		CHECK( uses[0].get_id() == token.get_id() );
	}
}

TEST_CASE(
	"a command_queue should wait for what a command depends on",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	auto &queue = fixture.get();
	const auto program = std::make_shared<cuda::mock_stream_program>();

	SECTION( "nothing for a token that stands for no command" )
	{
		REQUIRE_CALL(
			*program,
			launch(
				trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
			)
		);

		command cmd(program);
		cmd.bind_dependencies({command_token()});
		queue.submit(std::move(cmd));
	}

	SECTION( "a command of its own by running in order" )
	{
		ALLOW_CALL(
			*program,
			launch(
				trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
			)
		);

		cuda::stream_blocker blocker(queue.get_handle());
		const auto first = queue.submit(command(program));

		// Waiting for it on the calling thread would never return.
		command cmd(program);
		cmd.bind_dependencies({first});
		const auto second = queue.submit(std::move(cmd));

		CHECK_FALSE( second.is_complete() );

		blocker.release();
		second.wait();

		CHECK( first.is_complete() );
	}

	SECTION( "a command of another queue of this backend on the device" )
	{
		ALLOW_CALL(
			*program,
			launch(
				trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
			)
		);

		cuda::command_queue other(
			0,
			std::make_shared<cuda::pooled_event_recorder>(0)
		);
		cuda::stream_blocker blocker(other.get_handle());
		const auto first = other.submit(command(program));

		// Returns at once, and leaves the stream waiting for the other one.
		command cmd(program);
		cmd.bind_dependencies({first});
		const auto second = queue.submit(std::move(cmd));

		CHECK_FALSE( second.is_complete() );

		blocker.release();
		second.wait();

		CHECK( first.is_complete() );
	}

	SECTION( "a command of any other queue on the calling thread" )
	{
		const auto timeline = std::make_shared<cuda::mock_command_timeline>();

		// There is no way to wait for it on the device, so it has finished
		// by the time the work is issued.
		trompeloeil::sequence sequence;
		REQUIRE_CALL(*timeline, wait(5u))
			.IN_SEQUENCE(sequence);
		REQUIRE_CALL(
			*program,
			launch(
				trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
			)
		)
			.IN_SEQUENCE(sequence);

		command cmd(program);
		cmd.bind_dependencies({command_token(timeline, 5)});
		queue.submit(std::move(cmd));
	}
}

TEST_CASE(
	"a command_queue should finish a command that holds a buffer it did not "
	"allocate before returning",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	auto &queue = fixture.get();
	const auto program = std::make_shared<cuda::mock_stream_program>();
	const auto own = fixture.allocate();
	const auto foreign = std::make_shared<cuda::mock_buffer>();

	cuda::stream_probe probe;
	REQUIRE_CALL(
		*program,
		launch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.LR_SIDE_EFFECT(probe.issue(_4));

	command cmd(program);
	cmd.bind_outputs({foreign}).bind_inputs({own});
	const auto token = queue.submit(std::move(cmd));

	// Nobody would keep the memory of that buffer until the work ends, so
	// there is no work left by the time anyone could drop it.
	CHECK( probe.has_finished() );
	CHECK( token.is_empty() );
	CHECK( cuda::buffer::cast(*own).get_recorded_uses().empty() );
}

TEST_CASE(
	"a command_queue should finish the work of a program that failed before "
	"saying so",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	queue_fixture fixture;
	auto &queue = fixture.get();
	const auto program = std::make_shared<cuda::mock_stream_program>();
	const auto output = fixture.allocate();

	// Part of the work is issued before the program gives up.
	cuda::stream_probe probe;
	REQUIRE_CALL(
		*program,
		launch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
	)
		.LR_SIDE_EFFECT(probe.issue(_4))
		.THROW(std::runtime_error("out of something"));

	command cmd(program);
	cmd.bind_outputs({output});
	CHECK_THROWS_AS( queue.submit(std::move(cmd)), std::runtime_error );

	// No token came out of it, so nothing would ever wait for that work.
	CHECK( probe.has_finished() );
	CHECK( cuda::buffer::cast(*output).get_recorded_uses().empty() );
}

TEST_CASE(
	"casting a command_queue should reject one this backend did not make",
	"[command_queue]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	cuda::command_queue queue(
		0,
		std::make_shared<cuda::pooled_event_recorder>(0)
	);
	rexlib::command_queue &base = queue;

	CHECK( &cuda::command_queue::cast(base) == &queue );

	cuda::mock_command_queue foreign;
	CHECK_THROWS_AS(
		cuda::command_queue::cast(foreign),
		std::invalid_argument
	);

	const rexlib::command_queue &const_foreign = foreign;
	CHECK_THROWS_AS(
		cuda::command_queue::cast(const_foreign),
		std::invalid_argument
	);
}
