// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/philox.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"philox4x32_10 should reproduce the known answers of Random123",
	"[philox]"
)
{
	CHECK(
		philox4x32_10({{ 0, 0, 0, 0 }}, {{ 0, 0 }}) ==
		philox_counter_type{{
			0x6627e8d5u, 0xe169c58du, 0xbc57ac4cu, 0x9b00dbd8u
		}}
	);
	CHECK(
		philox4x32_10(
			{{ 0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu }},
			{{ 0xffffffffu, 0xffffffffu }}
		) ==
		philox_counter_type{{
			0x408f276du, 0x41c83b0eu, 0xa20bc7c6u, 0x6d5451fdu
		}}
	);
	CHECK(
		philox4x32_10(
			{{ 0x243f6a88u, 0x85a308d3u, 0x13198a2eu, 0x03707344u }},
			{{ 0xa4093822u, 0x299f31d0u }}
		) ==
		philox_counter_type{{
			0xd16cfe09u, 0x94fdccebu, 0x5001e420u, 0x24126ea1u
		}}
	);
}
