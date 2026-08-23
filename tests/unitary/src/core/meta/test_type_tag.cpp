// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/meta/type_tag.hpp>

#include <type_traits>

using namespace rex;

struct test_type {};

TEST_CASE( "type_tag should propagate the template argument to its type member", "[type_tag]" )
{
	CHECK( std::is_same<type_tag<test_type>::type, test_type>::value );
}
