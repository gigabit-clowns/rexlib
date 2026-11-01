// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/philox.hpp>
#include <backends/cpu/kernels/random_key.hpp>

#include <cstdint>

using namespace rexlib::cpu;

TEST_CASE(
	"derive_key should reproduce a known answer",
	"[random_key]"
)
{
	CHECK( derive_key(0, 0) == 0x1348e23f2dce73e5u );
	CHECK( derive_key(0x0123456789abcdefu, 5) == 0x3fe5d80eb1043319u );
}

TEST_CASE(
	"derive_key should take the first half of the key derivation block",
	"[random_key]"
)
{
	const std::uint64_t key = 0x0123456789abcdefu;
	const std::uint64_t data = 0x100000002u;

	const auto block = philox4x32_10(
		{{ 0x00000002u, 0x00000001u, 0, 1 }},
		{{ 0x89abcdefu, 0x01234567u }}
	);

	CHECK(
		derive_key(key, data) ==
		((static_cast<std::uint64_t>(block[1]) << 32) | block[0])
	);
}

TEST_CASE(
	"derive_key should differ from the draws of its parent key",
	"[random_key]"
)
{
	const std::uint64_t key = 42;
	const std::uint64_t data = 7;

	const auto draw = make_philox_halves(
		philox4x32_10(
			make_philox_counter(data, 0, philox_domain::draw),
			make_philox_key(key)
		)
	);

	CHECK( derive_key(key, data) != draw[0] );
}

TEST_CASE(
	"derive_key should depend on both the key and the value",
	"[random_key]"
)
{
	CHECK( derive_key(1, 0) == derive_key(1, 0) );
	CHECK( derive_key(1, 0) != derive_key(1, 1) );
	CHECK( derive_key(1, 0) != derive_key(2, 0) );
	CHECK( derive_key(0, 0) != 0 );
}
