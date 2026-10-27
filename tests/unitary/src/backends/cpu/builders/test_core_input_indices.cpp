// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/core_input_indices.hpp>

#include <rexlib/ops/rules/shape_rules.hpp>

#include <type_traits>
#include <utility>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"core_input_indices should select the inputs carrying core axes",
	"[core_input_indices]"
)
{
	STATIC_CHECK(
		std::is_same<
			core_input_indices<ops::keyed_axiswise_shape_rule>::type,
			std::index_sequence<0>
		>::value
	);
	STATIC_CHECK(
		std::is_same<
			core_input_indices<ops::matrix_multiply_shape_rule>::type,
			std::index_sequence<0, 1>
		>::value
	);
	STATIC_CHECK(
		std::is_same<
			core_input_indices<ops::binary_elementwise_shape_rule>::type,
			std::index_sequence<>
		>::value
	);
}
