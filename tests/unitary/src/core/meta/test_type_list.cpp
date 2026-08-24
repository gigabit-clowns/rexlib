// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/meta/type_list.hpp>

#include <type_traits>

using namespace rexlib;

struct test_type {};

TEST_CASE( "querying the size of a type_list should return the correct size", "[type_list]" )
{
	CHECK( type_list_size<type_list<float, int, float>>::value == 3);
	CHECK( type_list_size<type_list<int, test_type>>::value == 2);
	CHECK( type_list_size<type_list<test_type>>::value == 1);
	CHECK( type_list_size<type_list<>>::value == 0);
}

TEST_CASE( "indexing elements in a type_list should return the correct element", "[type_list]" )
{
	CHECK( std::is_same<type_list_element<0, type_list<float, int, float>>::type, float>::value );
	CHECK( std::is_same<type_list_element<0, type_list<test_type>>::type, test_type>::value );
	CHECK( std::is_same<type_list_element<1, type_list<test_type, int>>::type, int>::value );
}
