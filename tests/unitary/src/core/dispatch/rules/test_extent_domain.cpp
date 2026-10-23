// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/extent_domain.hpp>

#include <rexlib/core/dispatch/rules/positive_extent_domain.hpp>

#include <cstddef>
#include <string>

using namespace rexlib;

namespace
{

bool is_even(std::size_t extent)
{
	return extent % 2 == 0;
}

} // anonymous namespace

TEST_CASE(
	"extent_domain should answer through the membership test it was given",
	"[extent_domain]"
)
{
	const extent_domain domain(&is_even, "even");
	CHECK( domain.contains(0) );
	CHECK( domain.contains(4) );
	CHECK_FALSE( domain.contains(3) );
	CHECK( std::string(domain.get_description()) == "even" );
}

TEST_CASE(
	"make_extent_domain should build the domain a tag names",
	"[extent_domain]"
)
{
	const auto domain = make_extent_domain<positive_extent_domain>();
	CHECK_FALSE( domain.contains(0) );
	CHECK( domain.contains(1) );
	CHECK(
		std::string(domain.get_description()) ==
		positive_extent_domain::get_description()
	);
}
