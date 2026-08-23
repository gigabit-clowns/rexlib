// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/event_ticket.hpp>

#include "mock/mock_event_recorder.hpp"
#include "test_queue.hpp"

#include <stdexcept>
#include <utility>

using namespace xmipp4;

TEST_CASE(
	"a default constructed event_ticket should stand for a point already "
	"reached",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const cuda::event_ticket ticket;

	// Owning nothing has to read as nothing left to wait for, so that a block
	// with no points recorded against it is not held back for ever.
	CHECK_FALSE( static_cast<bool>(ticket) );
	CHECK( ticket.is_complete() );
	CHECK_NOTHROW( ticket.wait() );
	CHECK_NOTHROW( ticket.enqueue_wait(cuda::make_test_queue(0)) );
}

TEST_CASE(
	"an event_ticket should capture a point and give it back",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto queue = cuda::make_test_queue(0);

	REQUIRE_CALL(recorder, record(queue))
		.RETURN(7);
	REQUIRE_CALL(recorder, release(7u));

	{
		const cuda::event_ticket ticket(recorder, queue);
		CHECK( static_cast<bool>(ticket) );
	}
}

TEST_CASE(
	"an event_ticket should forward what is asked of the point it owns",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto queue = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);

	ALLOW_CALL(recorder, record(queue))
		.RETURN(7);
	ALLOW_CALL(recorder, release(7u));

	const cuda::event_ticket ticket(recorder, queue);

	SECTION( "whether it has been reached" )
	{
		REQUIRE_CALL(recorder, is_complete(7u))
			.RETURN(false);
		CHECK_FALSE( ticket.is_complete() );
	}

	SECTION( "waiting for it on the calling thread" )
	{
		REQUIRE_CALL(recorder, wait(7u));
		ticket.wait();
	}

	SECTION( "and deferring another queue until it is reached" )
	{
		REQUIRE_CALL(recorder, enqueue_wait(other, 7u));
		ticket.enqueue_wait(other);
	}
}

TEST_CASE(
	"an event_ticket should not give a point back that it no longer owns",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto queue = cuda::make_test_queue(0);

	ALLOW_CALL(recorder, record(queue))
		.RETURN(7);

	// Exactly once however the ticket is moved about: giving a point back
	// twice would let it be handed out while something still holds it.
	REQUIRE_CALL(recorder, release(7u));

	SECTION( "when it has been moved from" )
	{
		cuda::event_ticket source(recorder, queue);
		const cuda::event_ticket sink(std::move(source));

		CHECK_FALSE( static_cast<bool>(source) );
		CHECK( static_cast<bool>(sink) );

		// The moved from ticket owns nothing, so it reads as reached.
		CHECK( source.is_complete() );
	}

	SECTION( "when it has been moved from by assignment" )
	{
		cuda::event_ticket source(recorder, queue);
		cuda::event_ticket sink;
		sink = std::move(source);

		CHECK_FALSE( static_cast<bool>(source) );
		CHECK( static_cast<bool>(sink) );
	}

	SECTION( "when it has been reset" )
	{
		cuda::event_ticket ticket(recorder, queue);
		ticket.reset();

		CHECK_FALSE( static_cast<bool>(ticket) );

		// Resetting what owns nothing is not a second release.
		ticket.reset();
	}
}

TEST_CASE(
	"assigning over an event_ticket should give back the point it held",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto first = cuda::make_test_queue(0);
	const auto second = cuda::make_test_queue(1);

	ALLOW_CALL(recorder, record(first))
		.RETURN(7);
	ALLOW_CALL(recorder, record(second))
		.RETURN(8);

	cuda::event_ticket sink(recorder, first);
	cuda::event_ticket source(recorder, second);

	// The point being overwritten is the one that would otherwise be stranded.
	REQUIRE_CALL(recorder, release(7u));
	sink = std::move(source);

	CHECK( static_cast<bool>(sink) );

	REQUIRE_CALL(recorder, release(8u));
	sink.reset();
}

TEST_CASE(
	"swapping event_tickets should exchange the points they own",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto queue = cuda::make_test_queue(0);

	ALLOW_CALL(recorder, record(queue))
		.RETURN(7);

	cuda::event_ticket owning(recorder, queue);
	cuda::event_ticket empty;

	owning.swap(empty);

	CHECK_FALSE( static_cast<bool>(owning) );
	CHECK( static_cast<bool>(empty) );

	// Which one gives the point back follows the point, not the name.
	REQUIRE_CALL(recorder, release(7u));
	empty.reset();
}

TEST_CASE(
	"an event_ticket should own nothing when the point could not be captured",
	"[event_ticket]"
)
{
	cuda::mock_event_recorder recorder;
	const auto queue = cuda::make_test_queue(0);

	REQUIRE_CALL(recorder, record(queue))
		.THROW(std::runtime_error("no more events"));

	// Nothing was captured, so there is nothing to give back on the way out.
	FORBID_CALL(recorder, release(trompeloeil::_));

	CHECK_THROWS_AS(
		cuda::event_ticket(recorder, queue),
		std::runtime_error
	);
}
