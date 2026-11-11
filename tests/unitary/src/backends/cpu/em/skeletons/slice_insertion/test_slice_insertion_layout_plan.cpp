// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/slice_insertion_layout_plan.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

operand_descriptor make_descriptor(std::vector<std::size_t> extents)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32,
		nullptr
	);
}

const std::array<std::size_t, 5> core_ranks = {{ 3, 3, 2, 2, 1 }};

} // anonymous namespace

TEST_CASE(
	"slice_insertion_layout_plan should peel the cores off and read the "
	"extents from them",
	"[slice_insertion_layout_plan]"
)
{
	const std::array<operand_descriptor, 2> outputs = {
		make_descriptor({ 8, 8, 5 }), make_descriptor({ 8, 8, 5 })
	};
	const std::array<operand_descriptor, 5> inputs = {
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 3, 4, 3 }),
		make_descriptor({ 3, 3, 4 }),
		make_descriptor({ 3 })
	};

	const slice_insertion_layout_plan plan(
		make_span(outputs), make_span(inputs), make_span(core_ranks)
	);

	CHECK( plan.get_volume_extent() == 8 );
	CHECK( plan.get_slice_extent() == 4 );
	CHECK( plan.get_output_core(0).get_rank() == 3 );
	CHECK( plan.get_output_core(0).get_stride(0) == 40 );
	CHECK( plan.get_input_core(2).get_rank() == 2 );
	CHECK( plan.get_input_core(2).get_extent(1) == 3 );
	CHECK( plan.get_input_core(3).get_stride(0) == 4 );
	CHECK( plan.get_input_core(4).get_rank() == 1 );
	CHECK( plan.get_input_core(4).get_extent(0) == 3 );
}

TEST_CASE(
	"slice_insertion_layout_plan should reduce the batch axes the "
	"accumulators lack into them",
	"[slice_insertion_layout_plan]"
)
{
	const std::array<operand_descriptor, 2> outputs = {
		make_descriptor({ 8, 8, 5 }), make_descriptor({ 8, 8, 5 })
	};
	const std::array<operand_descriptor, 5> inputs = {
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 3, 4, 3 }),
		make_descriptor({ 3, 3, 4 }),
		make_descriptor({ 3 })
	};

	const slice_insertion_layout_plan plan(
		make_span(outputs), make_span(inputs), make_span(core_ranks)
	);

	CHECK( plan.get_kept_count() == 1 );
	CHECK( plan.get_reduced_count() == 3 );
	CHECK( plan.get_kept_layout().get_rank() == 0 );
	CHECK( plan.get_kept_layout().get_number_of_operands() == 7 );

	const auto &reduced = plan.get_reduced_layout();
	CHECK( reduced.get_rank() == 1 );
	CHECK( reduced.get_number_of_operands() == 5 );
	CHECK( reduced.get_strides(0)[0] == 0 );
	CHECK( reduced.get_strides(2)[0] == 12 );
	CHECK( reduced.get_strides(3)[0] == 12 );
	CHECK( reduced.get_strides(4)[0] == 0 );
}

TEST_CASE(
	"slice_insertion_layout_plan should keep the batch axes the accumulators "
	"carry",
	"[slice_insertion_layout_plan]"
)
{
	const std::array<operand_descriptor, 2> outputs = {
		make_descriptor({ 2, 1, 8, 8, 5 }), make_descriptor({ 2, 1, 8, 8, 5 })
	};
	const std::array<operand_descriptor, 5> inputs = {
		make_descriptor({ 2, 1, 8, 8, 5 }),
		make_descriptor({ 2, 1, 8, 8, 5 }),
		make_descriptor({ 3, 4, 3 }),
		make_descriptor({ 2, 3, 3, 4 }),
		make_descriptor({ 3 })
	};

	const slice_insertion_layout_plan plan(
		make_span(outputs), make_span(inputs), make_span(core_ranks)
	);

	CHECK( plan.get_kept_count() == 2 );
	CHECK( plan.get_reduced_count() == 3 );

	const auto &kept = plan.get_kept_layout();
	CHECK( kept.get_rank() == 1 );
	CHECK( kept.get_strides(0)[0] == 320 );
	CHECK( kept.get_strides(2)[0] == 0 );
	CHECK( kept.get_strides(3)[0] == 36 );
	CHECK( kept.get_strides(5)[0] == 320 );
	CHECK( kept.get_offset(5) == 0 );

	const auto &reduced = plan.get_reduced_layout();
	CHECK( reduced.get_rank() == 1 );
	CHECK( reduced.get_strides(0)[0] == 0 );
	CHECK( reduced.get_strides(2)[0] == 12 );
	CHECK( reduced.get_strides(3)[0] == 12 );
}

TEST_CASE(
	"slice_insertion_layout_plan should tell an output laid out as its "
	"accumulator from one that is not",
	"[slice_insertion_layout_plan]"
)
{
	const std::vector<std::size_t> extents = { 8, 8, 5 };
	const operand_descriptor shifted(
		strided_layout::make_custom_layout(
			make_span(extents),
			make_span(std::vector<std::ptrdiff_t>{ 40, 5, 1 }),
			7
		),
		numerical_type::float32,
		nullptr
	);
	const std::array<operand_descriptor, 2> outputs = {
		make_descriptor({ 8, 8, 5 }), shifted
	};
	const std::array<operand_descriptor, 5> inputs = {
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 4, 3 }),
		make_descriptor({ 3, 4 }),
		make_descriptor({ 3 })
	};

	const slice_insertion_layout_plan plan(
		make_span(outputs), make_span(inputs), make_span(core_ranks)
	);

	CHECK( plan.output_matches_accumulator(0) );
	CHECK_FALSE( plan.output_matches_accumulator(1) );
	CHECK( plan.get_kept_count() == 1 );
	CHECK( plan.get_reduced_count() == 1 );
}

TEST_CASE(
	"slice_insertion_layout_plan should reject an operand with fewer axes "
	"than its core",
	"[slice_insertion_layout_plan]"
)
{
	const std::array<operand_descriptor, 2> outputs = {
		make_descriptor({ 8, 8, 5 }), make_descriptor({ 8, 8, 5 })
	};
	const std::array<operand_descriptor, 5> inputs = {
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 8, 8, 5 }),
		make_descriptor({ 3 }),
		make_descriptor({ 3, 4 }),
		make_descriptor({ 3 })
	};

	CHECK_THROWS_AS(
		slice_insertion_layout_plan(
			make_span(outputs), make_span(inputs), make_span(core_ranks)
		),
		std::invalid_argument
	);
}
