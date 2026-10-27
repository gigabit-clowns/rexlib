// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/plans/batch_reduce_map_plan.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

using offset_list_type = std::vector<std::ptrdiff_t>;

operand_descriptor make_operand_descriptor(
	std::vector<std::size_t> extents,
	numerical_type data_type = numerical_type::float32
)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		data_type,
		nullptr
	);
}

/**
 * @brief The cores of an operation reading images and a key without core
 * axes.
 */
span<const core_dimensions_descriptor> keyed_cores() noexcept
{
	return shape_rule_descriptor_of<ops::keyed_axiswise_shape_rule>::get()
		.get_input_cores();
}

/**
 * @brief The offsets of every operand at every position of a layout, in
 * row-major order over its own axes.
 */
std::vector<offset_list_type> enumerate_offsets(const joint_layout &layout)
{
	const auto extents = layout.get_extents();
	const auto operand_count = layout.get_number_of_operands();

	std::vector<offset_list_type> result;
	std::vector<std::size_t> index(extents.size(), 0);
	for (std::size_t n = 0; n < layout.compute_element_count(); ++n)
	{
		offset_list_type offsets;
		for (std::size_t op = 0; op < operand_count; ++op)
		{
			const auto strides = layout.get_strides(op);
			auto offset = layout.get_offset(op);
			for (std::size_t axis = 0; axis < extents.size(); ++axis)
			{
				offset += static_cast<std::ptrdiff_t>(index[axis]) *
					strides[axis];
			}
			offsets.push_back(offset);
		}
		result.push_back(offsets);

		for (std::size_t axis = extents.size(); axis > 0; --axis)
		{
			if (++index[axis - 1] < extents[axis - 1])
			{
				break;
			}
			index[axis - 1] = 0;
		}
	}
	return result;
}

} // anonymous namespace

TEST_CASE(
	"batch_reduce_map_plan should lay out the operands of every layout",
	"[batch_reduce_map_plan]"
)
{
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 4, 5, 6 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 4, 5, 6 }),
		make_operand_descriptor({ 4 }, numerical_type::uint64)
	}};
	const std::vector<std::size_t> axes = { 2, 1 };

	const batch_reduce_map_plan plan(
		make_span(outputs),
		make_span(inputs),
		keyed_cores(),
		make_span(axes),
		2,
		multidimensional_index_tag(),
		linear_index_tag()
	);

	CHECK( plan.get_batch_count() == 4 );
	CHECK( plan.get_core_count() == 30 );
	CHECK( plan.get_frame().get_core_extents() ==
		std::vector<std::size_t>{ 5, 6 } );

	CHECK( plan.get_batch_layout().get_number_of_operands() == 4 );
	CHECK( plan.get_batch_layout().compute_element_count() == 4 );

	CHECK( plan.get_scalar_layout().get_rank() == 0 );
	CHECK( plan.get_scalar_layout().get_number_of_operands() == 3 );

	CHECK( plan.get_reduced_layout().get_number_of_operands() == 3 );
	CHECK( plan.get_reduced_layout().compute_element_count() == 30 );

	CHECK( plan.get_map_layout().get_number_of_operands() == 3 );
	CHECK( plan.get_map_layout().compute_element_count() == 30 );
}

TEST_CASE(
	"batch_reduce_map_plan should add no counter when the map is not "
	"indexed",
	"[batch_reduce_map_plan]"
)
{
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 4, 5, 6 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 4, 5, 6 }),
		make_operand_descriptor({}, numerical_type::uint64)
	}};
	const std::vector<std::size_t> axes = { 1, 2 };

	const batch_reduce_map_plan plan(
		make_span(outputs),
		make_span(inputs),
		keyed_cores(),
		make_span(axes),
		1,
		no_index_tag(),
		no_index_tag()
	);

	CHECK( plan.get_batch_layout().get_number_of_operands() == 3 );
	CHECK( plan.get_reduced_layout().get_number_of_operands() == 1 );
	CHECK( plan.get_map_layout().get_number_of_operands() == 2 );
}

TEST_CASE(
	"batch_reduce_map_plan should give the offsets to the batch layout alone",
	"[batch_reduce_map_plan]"
)
{
	const std::vector<std::size_t> extents = { 3, 4, 5 };
	const std::vector<std::ptrdiff_t> strides = { 20, 5, 1 };
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor(extents)
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		operand_descriptor(
			strided_layout::make_custom_layout(
				make_span(extents),
				make_span(strides),
				7
			),
			numerical_type::float32,
			nullptr
		),
		make_operand_descriptor({ 3 }, numerical_type::uint64)
	}};
	const std::vector<std::size_t> axes = { 1, 2 };

	const batch_reduce_map_plan plan(
		make_span(outputs),
		make_span(inputs),
		keyed_cores(),
		make_span(axes),
		2,
		multidimensional_index_tag(),
		linear_index_tag()
	);

	CHECK( plan.get_batch_layout().get_offset(0) == 7 );
	CHECK( plan.get_reduced_layout().get_offset(0) == 0 );
	CHECK( plan.get_map_layout().get_offset(1) == 0 );
}

TEST_CASE(
	"batch_reduce_map_plan should count the position of every element among "
	"those sharing its key",
	"[batch_reduce_map_plan]"
)
{
	// The image axes are 0 and 2, so the batch axis lies between them.
	const std::vector<std::size_t> frame = { 5, 3, 4 };
	const std::vector<std::size_t> axes = { 0, 2 };
	const auto key_extents = GENERATE(
		std::vector<std::size_t>{},
		std::vector<std::size_t>{ 1 },
		std::vector<std::size_t>{ 3 }
	);
	const bool shared_key = key_extents.empty() || key_extents[0] == 1;

	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor(frame)
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor(frame),
		make_operand_descriptor(key_extents, numerical_type::uint64)
	}};

	const batch_reduce_map_plan plan(
		make_span(outputs),
		make_span(inputs),
		keyed_cores(),
		make_span(axes),
		2,
		multidimensional_index_tag(),
		linear_index_tag()
	);

	// The batch layout holds value, key, output and counter, the map layout
	// output, value and counter.
	const auto batch = enumerate_offsets(plan.get_batch_layout());
	const auto map = enumerate_offsets(plan.get_map_layout());
	REQUIRE( batch.size() == 3 );
	REQUIRE( map.size() == 20 );

	std::size_t visited = 0;
	for (const auto &outer : batch)
	{
		for (const auto &inner : map)
		{
			const auto element = outer[2] + inner[0];
			const auto i0 = element / 12;
			const auto i1 = (element / 4) % 3;
			const auto i2 = element % 4;
			const auto expected = shared_key
				? element
				: i0 * 4 + i2;

			CHECK( outer[3] + inner[2] == expected );
			CHECK( outer[0] + inner[1] == element );
			CHECK( outer[1] == (shared_key ? 0 : i1) );
			++visited;
		}
	}
	CHECK( visited == 60 );
}

TEST_CASE(
	"batch_reduce_map_plan should reject a missing output, mismatched cores "
	"and inputs without core axes only",
	"[batch_reduce_map_plan]"
)
{
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 4, 5 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 4, 5 }),
		make_operand_descriptor({}, numerical_type::uint64)
	}};
	const std::vector<std::size_t> axes = { 1 };
	const auto cores = keyed_cores();
	const std::array<core_dimensions_descriptor, 2> empty_cores = {{
		cores[1],
		cores[1]
	}};

	CHECK_THROWS_AS(
		batch_reduce_map_plan(
			span<const operand_descriptor>(),
			make_span(inputs),
			cores,
			make_span(axes),
			1,
			no_index_tag(),
			no_index_tag()
		),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		batch_reduce_map_plan(
			make_span(outputs),
			make_span(inputs),
			make_span(cores.data(), 1),
			make_span(axes),
			1,
			no_index_tag(),
			no_index_tag()
		),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		batch_reduce_map_plan(
			make_span(outputs),
			make_span(inputs),
			make_span(empty_cores),
			make_span(axes),
			1,
			no_index_tag(),
			no_index_tag()
		),
		std::invalid_argument
	);
}
