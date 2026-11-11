// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/trailing_core.hpp>
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

operand_descriptor make_operand_descriptor(std::vector<std::size_t> extents)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32,
		nullptr
	);
}

/**
 * @brief Images, and a pair of coefficients per image: a core of their own.
 */
using affine_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>, extent_fixed<2>>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<1>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<2>>
	>,
	trailing_core
>;

span<const core_dimensions_descriptor> affine_cores() noexcept
{
	return shape_rule_descriptor_of<affine_shape_rule>::get()
		.get_input_cores();
}

const std::array<std::size_t, 1> mapped_images = {{ 0 }};

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
	"batched_map_plan should read an input of its own core once per batch "
	"element, and count over the core of the outputs",
	"[batched_map_plan]"
)
{
	// Two images of 8 by 5, each with a pair of coefficients.
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 2, 8, 5 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 2, 8, 5 }),
		make_operand_descriptor({ 2, 2 })
	}};

	const batched_map_plan plan(
		make_trailing_core_frame({ 2, 8, 5 }, 2),
		make_span(outputs),
		make_span(inputs),
		affine_cores(),
		make_span(mapped_images),
		linear_index_tag()
	);

	CHECK( plan.get_batch_count() == 2 );
	CHECK( plan.get_core_count() == 40 );
	CHECK( plan.get_input_core(0).get_rank() == 0 );

	const auto &core = plan.get_input_core(1);
	REQUIRE( core.get_rank() == 1 );
	CHECK( core.get_extent(0) == 2 );
	CHECK( core.get_stride(0) == 1 );

	// Both inputs, the output and the batch part of the counter.
	const auto batch = enumerate_offsets(plan.get_batch_layout());
	REQUIRE( batch.size() == 2 );
	CHECK( batch[0] == offset_list_type{ 0, 0, 0, 0 } );
	CHECK( batch[1] == offset_list_type{ 40, 2, 40, 0 } );

	// The output, the images and the core part of the counter.
	const auto map = enumerate_offsets(plan.get_map_layout());
	REQUIRE( map.size() == 40 );
	for (std::size_t i = 0; i < map.size(); ++i)
	{
		const auto position = static_cast<std::ptrdiff_t>(i);
		CHECK( map[i] == offset_list_type{ position, position, position } );
	}
}

TEST_CASE(
	"batched_map_plan should hand the map the coordinates of an element "
	"within the core",
	"[batched_map_plan]"
)
{
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 3, 4, 3 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 3, 4, 3 }),
		make_operand_descriptor({ 3, 2 })
	}};

	const batched_map_plan plan(
		make_trailing_core_frame({ 3, 4, 3 }, 2),
		make_span(outputs),
		make_span(inputs),
		affine_cores(),
		make_span(mapped_images),
		multidimensional_index_tag()
	);

	// The inputs and the output: no counter in the batch layout.
	CHECK( plan.get_batch_layout().get_number_of_operands() == 3 );
	// The output and the images, then one index operand per core axis.
	CHECK( plan.get_map_layout().get_number_of_operands() == 4 );
	CHECK( plan.get_map_layout().compute_element_count() == 12 );
}

TEST_CASE(
	"batched_map_plan should broadcast a mapped input without core axes "
	"against the core",
	"[batched_map_plan]"
)
{
	// Images of 4 by 5, and one value per image that the map walks.
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 2, 4, 5 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 2, 4, 5 }),
		make_operand_descriptor({ 2 })
	}};
	const auto cores =
		shape_rule_descriptor_of<ops::keyed_axiswise_shape_rule>::get()
			.get_input_cores();
	const std::array<std::size_t, 2> mapped = {{ 0, 1 }};
	const std::vector<std::size_t> axes = { 1, 2 };

	const batched_map_plan plan(
		named_core_frame({ 2, 4, 5 }, make_span(axes)),
		make_span(outputs),
		make_span(inputs),
		cores,
		make_span(mapped),
		no_index_tag()
	);

	const auto batch = enumerate_offsets(plan.get_batch_layout());
	REQUIRE( batch.size() == 2 );
	CHECK( batch[1] == offset_list_type{ 20, 1, 20 } );

	// The output, then both inputs; the second is the same at every element.
	const auto map = enumerate_offsets(plan.get_map_layout());
	REQUIRE( map.size() == 20 );
	for (std::size_t i = 0; i < map.size(); ++i)
	{
		const auto position = static_cast<std::ptrdiff_t>(i);
		CHECK( map[i] == offset_list_type{ position, position, 0 } );
	}
}

TEST_CASE(
	"batched_map_plan should reject a missing output, mismatched cores and "
	"an input read once per batch element led by a run",
	"[batched_map_plan]"
)
{
	const std::array<operand_descriptor, 1> outputs = {{
		make_operand_descriptor({ 4, 5 })
	}};
	const std::array<operand_descriptor, 2> inputs = {{
		make_operand_descriptor({ 4, 5 }),
		make_operand_descriptor({})
	}};
	const auto cores =
		shape_rule_descriptor_of<ops::keyed_axiswise_shape_rule>::get()
			.get_input_cores();
	const std::array<std::size_t, 1> mapped = {{ 0 }};
	const std::vector<std::size_t> axes = { 1 };

	CHECK_THROWS_AS(
		batched_map_plan(
			named_core_frame({ 4, 5 }, make_span(axes)),
			span<const operand_descriptor>(),
			make_span(inputs),
			cores,
			make_span(mapped),
			no_index_tag()
		),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		batched_map_plan(
			named_core_frame({ 4, 5 }, make_span(axes)),
			make_span(outputs),
			make_span(inputs),
			make_span(cores.data(), 1),
			make_span(mapped),
			no_index_tag()
		),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		batched_map_plan(
			named_core_frame({ 4, 5 }, make_span(axes)),
			make_span(outputs),
			make_span(inputs),
			cores,
			span<const std::size_t>(),
			no_index_tag()
		),
		std::invalid_argument
	);
}
