// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/linalg_core_layout_plan.hpp>

#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

operand_signature make_signature(std::vector<std::size_t> extents)
{
	return operand_signature(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32,
		nullptr
	);
}

} // namespace

TEST_CASE(
	"linalg_core_layout_plan::for_trailing_core splits whole-matrix operands "
	"into a rank zero batch",
	"[linalg_core_layout_plan]"
)
{
	const auto output = make_signature({ 2, 4 });
	const std::array<operand_signature, 2> inputs = {
		make_signature({ 2, 3 }), make_signature({ 3, 4 })
	};
	const std::array<operand_signature, 1> outputs = { output };

	const auto plan = linalg_core_layout_plan::for_trailing_core(
		make_span(outputs), make_span(inputs), 2, 2, 2
	);

	CHECK( plan.get_batch_layout().get_rank() == 0 );
	CHECK( plan.get_batch_layout().get_number_of_operands() == 3 );

	CHECK( plan.get_output_core().get_rank() == 2 );
	CHECK( plan.get_output_core().get_extent(0) == 2 );
	CHECK( plan.get_output_core().get_extent(1) == 4 );
	CHECK( plan.get_output_core().get_stride(0) == 4 );
	CHECK( plan.get_output_core().get_stride(1) == 1 );

	CHECK( plan.get_left_core().get_rank() == 2 );
	CHECK( plan.get_left_core().get_extent(0) == 2 );
	CHECK( plan.get_left_core().get_extent(1) == 3 );
	CHECK( plan.get_left_core().get_stride(0) == 3 );
	CHECK( plan.get_left_core().get_stride(1) == 1 );

	CHECK( plan.get_right_core().get_rank() == 2 );
	CHECK( plan.get_right_core().get_extent(0) == 3 );
	CHECK( plan.get_right_core().get_extent(1) == 4 );
	CHECK( plan.get_right_core().get_stride(0) == 4 );
	CHECK( plan.get_right_core().get_stride(1) == 1 );
}

TEST_CASE(
	"linalg_core_layout_plan::for_trailing_core broadcasts an operand with "
	"no batch axis of its own",
	"[linalg_core_layout_plan]"
)
{
	// Every operand's core rank is 2, so only output's leading axis (5) is
	// batch: its trailing {2, 4} is one core matrix per batch step. right's
	// rank equals its core rank, so it has no batch axis of its own and is
	// broadcast up to that one axis instead.
	const auto output = make_signature({ 5, 2, 4 });
	const std::array<operand_signature, 2> inputs = {
		make_signature({ 5, 2, 3 }), make_signature({ 3, 4 })
	};
	const std::array<operand_signature, 1> outputs = { output };

	const auto plan = linalg_core_layout_plan::for_trailing_core(
		make_span(outputs), make_span(inputs), 2, 2, 2
	);

	const auto &batch = plan.get_batch_layout();
	CHECK( batch.get_number_of_operands() == 3 );
	CHECK( batch.get_rank() == 1 );
	CHECK( batch.get_extents()[0] == 5 );

	// output and left keep their own contiguous batch stride...
	CHECK( batch.get_strides(0)[0] == 8 );
	CHECK( batch.get_strides(1)[0] == 6 );

	// ...while right, having contributed no batch axis, is broadcast: every
	// batch index reads the same element.
	CHECK( batch.get_strides(2)[0] == 0 );

	CHECK( plan.get_right_core().get_extent(0) == 3 );
	CHECK( plan.get_right_core().get_extent(1) == 4 );
}

TEST_CASE(
	"linalg_core_layout_plan::for_trailing_core rejects an operand without "
	"enough axes for its core",
	"[linalg_core_layout_plan]"
)
{
	const auto output = make_signature({ 2 });
	const std::array<operand_signature, 2> inputs = {
		make_signature({ 2, 3 }), make_signature({ 3 })
	};
	const std::array<operand_signature, 1> outputs = { output };

	CHECK_THROWS_AS(
		linalg_core_layout_plan::for_trailing_core(
			make_span(outputs), make_span(inputs), 1, 2, 2
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"linalg_core_layout_plan::for_named_axis pulls a non-trailing axis out, "
	"keeping the rest in order",
	"[linalg_core_layout_plan]"
)
{
	const auto output = make_signature({ 3, 4 });
	const std::array<operand_signature, 2> inputs = {
		make_signature({ 3, 4 }), make_signature({ 3, 4 })
	};
	const std::array<operand_signature, 1> outputs = { output };

	const auto plan =
		linalg_core_layout_plan::for_named_axis(
			make_span(outputs), make_span(inputs), 0
		);

	CHECK( plan.get_output_core().get_rank() == 1 );
	CHECK( plan.get_output_core().get_extent(0) == 3 );
	CHECK( plan.get_output_core().get_stride(0) == 4 );

	const auto &batch = plan.get_batch_layout();
	CHECK( batch.get_rank() == 1 );
	CHECK( batch.get_extents()[0] == 4 );
	CHECK( batch.get_strides(0)[0] == 1 );
}

TEST_CASE(
	"linalg_core_layout_plan::for_named_axis broadcasts an operand before "
	"slicing its core axis out of it",
	"[linalg_core_layout_plan]"
)
{
	const auto output = make_signature({ 3, 4 });
	const std::array<operand_signature, 2> inputs = {
		make_signature({ 3, 4 }), make_signature({ 1, 4 })
	};
	const std::array<operand_signature, 1> outputs = { output };

	const auto plan =
		linalg_core_layout_plan::for_named_axis(
			make_span(outputs), make_span(inputs), 0
		);

	// right was broadcast to {3, 4} first, so its core axis reads the same
	// row for every batch index.
	CHECK( plan.get_right_core().get_rank() == 1 );
	CHECK( plan.get_right_core().get_extent(0) == 3 );
	CHECK( plan.get_right_core().get_stride(0) == 0 );

	CHECK( plan.get_batch_layout().get_strides(2)[0] == 1 );
}
