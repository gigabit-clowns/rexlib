// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/buffer.hpp>

#include <hardware/caching_allocator/caching_memory_allocator.hpp>

#include "mock/mock_buffer.hpp"
#include "test_caching_allocator.hpp"
#include "test_queue.hpp"

#include <config.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace xmipp4;

namespace
{

constexpr std::size_t alignment =
	XMIPP4_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
constexpr std::size_t min_heap =
	XMIPP4_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES;

cuda::buffer& allocate_buffer(
	cuda::caching_allocator_fixture &fixture,
	const cuda::queue_handle &queue,
	std::shared_ptr<xmipp4::buffer> &owner
)
{
	owner = fixture.get().allocate(1024, alignment, queue);
	return cuda::buffer::cast(*owner);
}

} // namespace

TEST_CASE(
	"a fresh buffer should have no queues recorded against it",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	std::shared_ptr<xmipp4::buffer> owner;
	auto &buf = allocate_buffer(fixture, cuda::make_test_queue(0), owner);

	CHECK( buf.get_recorded_queues().empty() );
}

TEST_CASE(
	"a buffer should record the queues that could still be using it",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto own = cuda::make_test_queue(0);
	const auto first = cuda::make_test_queue(1);
	const auto second = cuda::make_test_queue(2);

	std::shared_ptr<xmipp4::buffer> owner;
	auto &buf = allocate_buffer(fixture, own, owner);

	SECTION( "one entry per queue" )
	{
		buf.record_use(first);
		buf.record_use(second);

		const auto queues = buf.get_recorded_queues();
		REQUIRE( queues.size() == 2 );
		CHECK( std::count(queues.begin(), queues.end(), first) == 1 );
		CHECK( std::count(queues.begin(), queues.end(), second) == 1 );
	}

	SECTION( "counting a queue only once however often it is given work" )
	{
		buf.record_use(first);
		buf.record_use(first);
		buf.record_use(first);

		const auto queues = buf.get_recorded_queues();
		REQUIRE( queues.size() == 1 );
		CHECK( queues[0] == first );
	}

	SECTION( "but never its own, which it is already ordered against" )
	{
		buf.record_use(own);

		// That queue runs in order, so whatever it is given after this cannot
		// overlap with the work being dropped. Waiting for it would be waiting
		// for nothing.
		CHECK( buf.get_recorded_queues().empty() );
	}
}

TEST_CASE(
	"a buffer used on another queue should not be handed out again until that "
	"queue has caught up",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);

	// Nothing counts as finished until a test says so.
	fixture.get_blocks().set_reached_by_default(false);

	// A whole heap at a time, so that the block being held back is the only
	// thing that could serve the next request.
	void *first_data = nullptr;
	{
		auto owner = fixture.get().allocate(min_heap, alignment, own);
		first_data = cuda::buffer::cast(*owner).get_device_ptr();
		cuda::buffer::cast(*owner).record_use(other);
	}

	// Held back rather than given straight to the pool, so the next request
	// has to be served out of memory taken from the driver instead.
	auto second = fixture.get().allocate(min_heap, alignment, own);
	CHECK( cuda::buffer::cast(*second).get_device_ptr() != first_data );
	CHECK( fixture.get_blocks().get_arena().get_used_bytes() == 2 * min_heap );

	// Once the queue that was using it has caught up, it goes back into the
	// pool like any other block.
	second.reset();
	fixture.get_blocks().reach(1);

	FORBID_CALL(fixture.get_blocks().get_source(), allocate(trompeloeil::_));
	auto third = fixture.get().allocate(min_heap, alignment, own);
	auto fourth = fixture.get().allocate(min_heap, alignment, own);
}

TEST_CASE(
	"a buffer used only on its own queue should go straight back",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto own = cuda::make_test_queue(0);

	fixture.get_blocks().set_reached_by_default(false);

	// Nothing has to be waited for, so nothing is captured.
	FORBID_CALL(fixture.get_blocks().get_recorder(), record(trompeloeil::_));

	void *first_data = nullptr;
	{
		std::shared_ptr<xmipp4::buffer> owner;
		auto &buf = allocate_buffer(fixture, own, owner);
		first_data = buf.get_device_ptr();
		buf.record_use(own);
	}

	auto second = fixture.get().allocate(1024, alignment, own);
	CHECK( cuda::buffer::cast(*second).get_device_ptr() == first_data );
}

TEST_CASE(
	"casting a buffer should reject one this backend did not make",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	std::shared_ptr<xmipp4::buffer> owner;
	auto &buf = allocate_buffer(fixture, cuda::make_test_queue(0), owner);

	CHECK( &cuda::buffer::cast(*owner) == &buf );

	cuda::mock_buffer foreign;
	CHECK_THROWS_AS( cuda::buffer::cast(foreign), std::invalid_argument );

	const auto &const_foreign = foreign;
	CHECK_THROWS_AS(
		cuda::buffer::cast(const_foreign),
		std::invalid_argument
	);
}
