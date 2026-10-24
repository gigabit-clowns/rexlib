// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_transform.hpp>

#include <rexlib/core/operation/type_rule/real_type_transform.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"make_type_transform should erase both halves a transform tag names",
	"[type_transform]"
)
{
	const auto transform = make_type_transform<real_type_transform>();

	CHECK( transform.apply(numerical_type::complex_float32) ==
	       numerical_type::float32 );
	CHECK( &transform.get_domain() ==
	       &real_type_transform::domain_type::get() );
	CHECK( std::string(transform.get_relation()) ==
	       "the real counterpart of" );
}

TEST_CASE(
	"type_transform should store what it is constructed from",
	"[type_transform]"
)
{
	const auto &domain = numerical_type_domain::all();
	const type_transform transform(
		&real_type_transform::apply,
		domain,
		"a relation"
	);

	CHECK( transform.apply(numerical_type::complex_float64) ==
	       numerical_type::float64 );
	CHECK( &transform.get_domain() == &domain );
	CHECK( std::string(transform.get_relation()) == "a relation" );
}
