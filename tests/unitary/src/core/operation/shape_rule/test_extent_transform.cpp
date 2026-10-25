// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/shape_rule/extent_transform.hpp>

#include <rexlib/core/operation/shape_rule/same_extent_transform.hpp>

#include "fixtures/doubled_extent_transform.hpp"

#include <string>

using namespace rexlib;

TEST_CASE(
	"make_extent_transform should erase the identity",
	"[extent_transform]"
)
{
	const auto transform = make_extent_transform<same_extent_transform>();

	CHECK( transform.apply(7) == 7 );
	CHECK( std::string(transform.get_relation()) == "the extent of" );
	CHECK( transform.is_identity() );
}

TEST_CASE(
	"make_extent_transform should erase any other transform",
	"[extent_transform]"
)
{
	const auto transform =
		make_extent_transform<test::doubled_extent_transform>();

	CHECK( transform.apply(7) == 14 );
	CHECK( std::string(transform.get_relation()) == "twice the extent of" );
	CHECK_FALSE( transform.is_identity() );
}

TEST_CASE(
	"extent_transform should store what it is constructed from",
	"[extent_transform]"
)
{
	const extent_transform transform(
		&test::doubled_extent_transform::apply,
		"a relation",
		true
	);

	CHECK( transform.apply(3) == 6 );
	CHECK( std::string(transform.get_relation()) == "a relation" );
	CHECK( transform.is_identity() );
}
