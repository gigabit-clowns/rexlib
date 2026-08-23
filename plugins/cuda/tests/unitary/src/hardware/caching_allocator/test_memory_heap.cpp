// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_heap.hpp>

#include "mock/mock_memory_source.hpp"

#include <xmipp4/core/memory/align.hpp>

#include <cstddef>
#include <new>
#include <stdexcept>

using namespace xmipp4;

namespace
{

/// Stands in for whatever the driver would have handed out. Aligned past what
/// the sources below promise, so that a test can pick a base that is not, and
/// deliberately no further: MSVC refuses to link a section aligned beyond the
/// page the image is laid out in.
alignas(1024) std::byte storage[8192];

} // namespace

TEST_CASE(
	"memory_heap should take a region from its source and give it back",
	"[memory_heap]"
)
{
	cuda::mock_memory_source source;
	auto *region = storage;

	ALLOW_CALL(source, get_base_alignment())
		.RETURN(256);
	REQUIRE_CALL(source, allocate(4096u))
		.RETURN(region);
	REQUIRE_CALL(source, deallocate(region));

	const cuda::memory_heap heap(source, 4096, 256);

	CHECK( heap.get_data() == region );
	CHECK( heap.get_size() == 4096 );
}

TEST_CASE(
	"memory_heap should align its base beyond what its source guarantees",
	"[memory_heap]"
)
{
	cuda::mock_memory_source source;

	// Aligned to what the source promises, but not to what the heap does, so
	// that the base has to move to make up the difference.
	auto *region = storage + 256;

	ALLOW_CALL(source, get_base_alignment())
		.RETURN(256);

	// Moving the base forwards eats into the region, so the difference between
	// the two alignments has to be paid for up front.
	REQUIRE_CALL(source, allocate(512u + 1024u - 256u))
		.RETURN(region);
	ALLOW_CALL(source, deallocate(region));

	const cuda::memory_heap heap(source, 512, 1024);

	CHECK( is_aligned(heap.get_data(), 1024) );
	CHECK( heap.get_size() == 512 );

	// Every usable byte still has to lie inside what was actually taken.
	const auto first = reinterpret_cast<std::uintptr_t>(heap.get_data());
	const auto region_first = reinterpret_cast<std::uintptr_t>(region);
	CHECK( first >= region_first );
	CHECK( first + heap.get_size() <= region_first + 512 + 1024 - 256 );
}

TEST_CASE(
	"memory_heap should not over-allocate when its source already aligns "
	"strictly enough",
	"[memory_heap]"
)
{
	cuda::mock_memory_source source;
	auto *region = storage;

	ALLOW_CALL(source, get_base_alignment())
		.RETURN(1024);
	REQUIRE_CALL(source, allocate(1024u))
		.RETURN(region);
	ALLOW_CALL(source, deallocate(region));

	const cuda::memory_heap heap(source, 1024, 256);

	CHECK( heap.get_data() == region );
}

TEST_CASE(
	"memory_heap should refuse to be empty",
	"[memory_heap]"
)
{
	cuda::mock_memory_source source;

	ALLOW_CALL(source, get_base_alignment())
		.RETURN(256);
	FORBID_CALL(source, allocate(trompeloeil::_));

	CHECK_THROWS_AS(
		cuda::memory_heap(source, 0, 256),
		std::invalid_argument
	);
}

TEST_CASE(
	"memory_heap should propagate an exhausted source without giving anything "
	"back",
	"[memory_heap]"
)
{
	cuda::mock_memory_source source;

	ALLOW_CALL(source, get_base_alignment())
		.RETURN(256);
	REQUIRE_CALL(source, allocate(2048u))
		.THROW(std::bad_alloc());
	FORBID_CALL(source, deallocate(trompeloeil::_));

	CHECK_THROWS_AS(
		cuda::memory_heap(source, 2048, 256),
		std::bad_alloc
	);
}
