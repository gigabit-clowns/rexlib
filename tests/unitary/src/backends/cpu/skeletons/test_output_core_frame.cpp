// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/skeletons/output_core_frame.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>
#include <rexlib/ops/linalg/matmul_operation.hpp>

#include <array>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

std::array<operand_descriptor, 1> make_output(
	std::vector<std::size_t> extents
)
{
	return {{
		operand_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32,
			nullptr
		)
	}};
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

} // anonymous namespace

TEST_CASE(
	"make_output_core_frame should take the trailing axes of the outputs "
	"as their core",
	"[output_core_frame]"
)
{
	const ops::matmul_operation operation;
	const auto outputs = make_output({ 2, 8, 5 });

	const auto frame = make_output_core_frame(operation, make_span(outputs));

	CHECK(
		to_vector(frame.get_core_axes()) == std::vector<std::size_t>{ 1, 2 }
	);
	CHECK( to_vector(frame.get_batch_axes()) == std::vector<std::size_t>{ 0 } );
}

TEST_CASE(
	"make_output_core_frame should take the axes the operation names as the "
	"core of the outputs",
	"[output_core_frame]"
)
{
	const em::standardize_background_operation operation(
		ops::axis_list{ 2, 0 },
		1,
		-3,
		3
	);
	const auto outputs = make_output({ 3, 4, 5 });

	const auto frame = make_output_core_frame(operation, make_span(outputs));

	CHECK(
		to_vector(frame.get_core_axes()) == std::vector<std::size_t>{ 0, 2 }
	);
	CHECK( to_vector(frame.get_batch_axes()) == std::vector<std::size_t>{ 1 } );
}
