// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/system/page_prefetch.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/system/host.hpp>

#include <vector>

using namespace rexlib;

TEST_CASE( "a memory range is the stretch it was built from",
	"[page_prefetch]" )
{
	float values[4];
	const memory_range range(values + 1, 3 * sizeof(float));

	CHECK( range.get_address() == static_cast<void*>(values + 1) );
	CHECK( range.get_size() == 3 * sizeof(float) );
}

TEST_CASE( "stretches of memory are advised before they are read",
	"[page_prefetch]" )
{
	const auto page = get_page_size();
	host_buffer memory(3 * page, page);
	auto *first = static_cast<byte*>(memory.get_host_ptr());

	// Advice leaves nothing a test can observe, so what is asserted is that
	// memory survives being advised what the contract allows: stretches that
	// start on a page and stay within what is mapped.
	SECTION( "a batch of no stretch is advised" )
	{
		const std::vector<memory_range> ranges;

		REQUIRE_NOTHROW( prefetch_pages(make_span(ranges)) );
	}

	SECTION( "one whole stretch is advised" )
	{
		const std::vector<memory_range> ranges = {{first, 3 * page}};

		REQUIRE_NOTHROW( prefetch_pages(make_span(ranges)) );
	}

	SECTION( "a stretch of less than a page is advised" )
	{
		const std::vector<memory_range> ranges = {{first, 8}};

		REQUIRE_NOTHROW( prefetch_pages(make_span(ranges)) );
	}

	SECTION( "a stretch of no bytes is advised" )
	{
		const std::vector<memory_range> ranges = {{first + page, 0}};

		REQUIRE_NOTHROW( prefetch_pages(make_span(ranges)) );
	}

	SECTION( "stretches of different memory are advised at once" )
	{
		host_buffer other(page, page);
		const std::vector<memory_range> ranges = {
			{first, page},
			{first + 2 * page, page},
			{other.get_host_ptr(), page}
		};

		REQUIRE_NOTHROW( prefetch_pages(make_span(ranges)) );
	}
}
