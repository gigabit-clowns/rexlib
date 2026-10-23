// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/contract/operand_specification.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;

namespace
{

using shape_type = operand_specification::shape_type;

} // anonymous namespace

TEST_CASE(
	"Default constructed operand_specification should have no axes and an "
	"unknown data type",
	"[operand_specification]"
)
{
	const operand_specification specification;
	CHECK( specification.get_shape().empty() );
	CHECK( specification.get_data_type() == numerical_type::unknown );
}

TEST_CASE(
	"Constructing an operand_specification should store its shape and data "
	"type",
	"[operand_specification]"
)
{
	const operand_specification specification(
		shape_type{ 4, 8 },
		numerical_type::float32
	);
	CHECK( specification.get_shape() == shape_type{ 4, 8 } );
	CHECK( specification.get_data_type() == numerical_type::float32 );
}

TEST_CASE(
	"Setting the shape and data type of an operand_specification should "
	"replace them",
	"[operand_specification]"
)
{
	operand_specification specification(
		shape_type{ 4 },
		numerical_type::int8
	);

	specification.set_shape(shape_type{ 2, 3, 5 });
	specification.set_data_type(numerical_type::complex_float64);

	CHECK( specification.get_shape() == shape_type{ 2, 3, 5 } );
	CHECK( specification.get_data_type() == numerical_type::complex_float64 );
}

TEST_CASE(
	"operand_specification-s should compare equal only when both their "
	"shapes and data types are",
	"[operand_specification]"
)
{
	const operand_specification reference(
		shape_type{ 4, 8 },
		numerical_type::float32
	);

	CHECK( reference == operand_specification(
		shape_type{ 4, 8 },
		numerical_type::float32
	) );
	CHECK( reference != operand_specification(
		shape_type{ 8, 4 },
		numerical_type::float32
	) );
	CHECK( reference != operand_specification(
		shape_type{ 4, 8 },
		numerical_type::float64
	) );
}

TEST_CASE(
	"operand_specification should take the extents and data type of a "
	"descriptor, whatever its strides",
	"[operand_specification]"
)
{
	const std::vector<std::size_t> extents = { 3, 5 };
	const std::vector<std::ptrdiff_t> strides = { 1, 3 };
	const array_descriptor descriptor(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			7
		),
		numerical_type::uint16
	);

	const auto specification =
		operand_specification::from_descriptor(descriptor);

	CHECK( specification.get_shape() == shape_type{ 3, 5 } );
	CHECK( specification.get_data_type() == numerical_type::uint16 );
}
