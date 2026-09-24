// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/memory_mapping/image_prefetch_policy.hpp>

#include <rexlib/core/system/host.hpp>

#include <cstddef>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "a policy reports what it was constructed with",
	"[image_prefetch_policy]" )
{
	const image_prefetch_policy policy(64, 4096, 16);

	CHECK( policy.get_gap_tolerance() == 64 );
	CHECK( policy.get_byte_budget() == 4096 );
	CHECK( policy.get_page_size() == 16 );
}

TEST_CASE( "the default policy scales its tolerance with the region",
	"[image_prefetch_policy]" )
{
	SECTION( "a region wider than the cap is capped" )
	{
		const auto policy =
			make_prefetch_policy(4 * default_prefetch_gap_cap);

		CHECK( policy.get_gap_tolerance() == default_prefetch_gap_cap );
	}

	SECTION( "a region narrower than the cap sets the tolerance" )
	{
		// Floored at a page, so the region has to be wider than one to be
		// what decides it.
		const std::size_t span = default_prefetch_gap_cap / 2;
		const auto policy = make_prefetch_policy(span);

		CHECK( policy.get_gap_tolerance() <= default_prefetch_gap_cap );
		CHECK( policy.get_gap_tolerance() >= span );
	}

	SECTION( "it takes the page size of the machine" )
	{
		// One plane of 3x4 float32 values.
		const auto policy = make_prefetch_policy(48);

		CHECK( policy.get_page_size() == get_page_size() );
		CHECK( policy.get_byte_budget() == default_prefetch_budget );
	}
}
