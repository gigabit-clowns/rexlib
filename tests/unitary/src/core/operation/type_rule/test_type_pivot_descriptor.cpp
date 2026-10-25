// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_pivot_descriptor.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>

using namespace rexlib;

TEST_CASE(
	"type_pivot_descriptor should store where its pivot comes from",
	"[type_pivot_descriptor]"
)
{
	const auto &domain = floating_point_type_domain::get();
	const type_pivot_descriptor descriptor(
		type_pivot_source::output,
		2,
		1,
		domain
	);

	CHECK( descriptor.get_source() == type_pivot_source::output );
	CHECK( descriptor.get_index() == 2 );
	CHECK( descriptor.get_default_pivot() == 1 );
	CHECK( &descriptor.get_domain() == &domain );
}

TEST_CASE(
	"type_pivot_descriptor should stay assignable",
	"[type_pivot_descriptor]"
)
{
	type_pivot_descriptor descriptor(
		type_pivot_source::input,
		0,
		-1,
		any_type_domain::get()
	);
	const type_pivot_descriptor other(
		type_pivot_source::operation,
		1,
		-1,
		complex_type_domain::get()
	);

	descriptor = other;

	CHECK( descriptor.get_source() == type_pivot_source::operation );
	CHECK( descriptor.get_index() == 1 );
	CHECK( &descriptor.get_domain() == &complex_type_domain::get() );
}
