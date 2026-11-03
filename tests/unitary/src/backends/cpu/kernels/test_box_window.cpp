// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/box_window.hpp>

#include <array>
#include <cstdint>

using namespace rexlib::cpu;

TEST_CASE(
	"box_window should place each sample on its nearest voxel with weight 1",
	"[box_window]"
)
{
	const box_window window;
	const std::array<float, 3> coordinates = {{ 2.3f, 2.7f, -2.3f }};
	std::array<std::int32_t, 3> first{};
	std::array<float, 3> weights{};

	window.place(coordinates.data(), 3, first.data(), weights.data(), 3);

	CHECK( window.get_tap_count() == 1 );
	CHECK( first == std::array<std::int32_t, 3>{{ 2, 3, -2 }} );
	CHECK( weights == std::array<float, 3>{{ 1, 1, 1 }} );
}

TEST_CASE(
	"box_window should send a sample halfway between voxels away from 0",
	"[box_window]"
)
{
	const box_window window;
	const std::array<double, 4> coordinates = {{ 2.5, -2.5, 0.5, -0.5 }};
	std::array<std::int32_t, 4> first{};
	std::array<double, 4> weights{};

	window.place(coordinates.data(), 4, first.data(), weights.data(), 4);

	CHECK( first == std::array<std::int32_t, 4>{{ 3, -3, 1, -1 }} );
}
