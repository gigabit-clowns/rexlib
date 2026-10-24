// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_slot_descriptor.hpp>

#include <rexlib/core/operation/type_rule/complex_type_transform.hpp>

#include <string>

using namespace rexlib;

TEST_CASE(
	"type_slot_descriptor should store how its operand follows a pivot",
	"[type_slot_descriptor]"
)
{
	const auto &domain = complex_type_transform::domain_type::get();
	const type_slot_descriptor descriptor(
		3,
		make_type_transform<complex_type_transform>()
	);

	CHECK( descriptor.get_pivot_index() == 3 );
	CHECK( descriptor.get_transform().apply(numerical_type::float32) ==
	       numerical_type::complex_float32 );
	CHECK( &descriptor.get_transform().get_domain() == &domain );
	CHECK( std::string(descriptor.get_transform().get_relation()) ==
	       "the complex counterpart of" );
}
