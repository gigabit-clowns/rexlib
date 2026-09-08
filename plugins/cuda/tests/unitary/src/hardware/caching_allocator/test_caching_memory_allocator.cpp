// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/caching_memory_allocator.hpp>

#include <hardware/caching_allocator/buffer.hpp>

#include "mock/mock_memory_resource.hpp"
#include "test_caching_allocator.hpp"
#include "test_queue.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/memory/align.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace rexlib;

namespace
{

constexpr std::size_t alignment =
	REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
constexpr std::size_t min_heap =
	REXLIB_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES;

} // namespace

TEST_CASE(
	"caching_memory_allocator should report what it is backed by",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	CHECK( &fixture.get().get_memory_resource() == &fixture.get_resource() );

	// Whatever the blocks underneath can be aligned to is what a buffer can
	// be, since a buffer is one of those blocks.
	CHECK(
		fixture.get().get_max_alignment() ==
		fixture.get_blocks().get().get_max_alignment()
	);
}

TEST_CASE(
	"caching_memory_allocator should refuse to be built without blocks to "
	"hand out",
	"[caching_memory_allocator]"
)
{
	cuda::mock_memory_resource resource;

	CHECK_THROWS_AS(
		cuda::caching_memory_allocator(resource, nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"a caching_memory_allocator should work out the queue a hint names",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	SECTION( "taking no hint to mean no queue in particular" )
	{
		// A buffer that named no queue can be handed to any of them, so what
		// is handed out belongs to none.
		FORBID_CALL(
			fixture.get_blocks().get_recorder(),
			record(trompeloeil::_)
		);

		auto buf = fixture.get().allocate(1024, alignment, nullptr);
		REQUIRE( buf != nullptr );

		auto other = fixture.get().allocate(1024, alignment, nullptr);
		REQUIRE( other != nullptr );
	}

	SECTION( "and refusing a queue this backend did not make" )
	{
		class foreign_queue final : public rexlib::command_queue
		{
		public:
			void submit(const rexlib::command&) override {}
			void signal(rexlib::event&) override {}
			void wait(const rexlib::event&) override {}
		};

		// There is no stream behind it, so there is nothing to order the
		// buffer against and nothing to record a point on.
		foreign_queue queue;
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, alignment, &queue),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should describe the block it "
	"was cut from",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	auto buf = fixture.get().allocate(1000, alignment, cuda::make_test_queue(0));

	REQUIRE( buf != nullptr );
	CHECK( &buf->get_memory_resource() == &fixture.get_resource() );

	// Rounding up is what keeps the remainder of a block usable, so the size
	// the buffer reports is the rounded one rather than what was asked for.
	CHECK( buf->get_size() == align_ceil(std::size_t(1000), alignment) );
	CHECK( is_aligned(cuda::buffer::cast(*buf).get_device_ptr(), alignment) );
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should only offer a host "
	"pointer when the host can reach it",
	"[caching_memory_allocator]"
)
{
	const auto queue = cuda::make_test_queue(0);

	SECTION( "not when it is device local" )
	{
		cuda::caching_allocator_fixture fixture(false);

		auto buf = fixture.get().allocate(1024, alignment, queue);
		CHECK( buf->get_host_ptr() == nullptr );
		CHECK( cuda::buffer::cast(*buf).get_device_ptr() != nullptr );
	}

	SECTION( "but yes when it is page locked host memory" )
	{
		cuda::caching_allocator_fixture fixture(true);

		auto buf = fixture.get().allocate(1024, alignment, queue);
		CHECK( buf->get_host_ptr() != nullptr );
		CHECK(
			buf->get_host_ptr() == cuda::buffer::cast(*buf).get_device_ptr()
		);
	}
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should give its block back",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);

	void *first = nullptr;
	{
		auto buf = fixture.get().allocate(min_heap, alignment, queue);
		first = cuda::buffer::cast(*buf).get_device_ptr();
	}

	// Only reachable again if dropping the buffer put the block back, since a
	// whole heap's worth leaves nothing else to cut this out of.
	FORBID_CALL(fixture.get_blocks().get_source(), allocate(trompeloeil::_));
	auto second = fixture.get().allocate(min_heap, alignment, queue);
	CHECK( cuda::buffer::cast(*second).get_device_ptr() == first );
}

TEST_CASE(
	"caching_memory_allocator should pass a trim down to its blocks",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	fixture.get().allocate(1024, alignment, cuda::make_test_queue(0));
	REQUIRE( fixture.get_blocks().get_arena().get_used_bytes() == min_heap );

	CHECK( fixture.get().trim() == min_heap );
	CHECK( fixture.get_blocks().get_arena().get_region_count() == 0 );
}
