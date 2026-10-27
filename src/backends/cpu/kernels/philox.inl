// SPDX-License-Identifier: GPL-3.0-only

#include "philox.hpp"

#include <rexlib/core/platform/constexpr.hpp>

namespace rexlib
{
namespace cpu
{

namespace detail
{

REXLIB_CONST_CONSTEXPR std::uint32_t philox_multiplier_0 = 0xD2511F53u;
REXLIB_CONST_CONSTEXPR std::uint32_t philox_multiplier_1 = 0xCD9E8D57u;
REXLIB_CONST_CONSTEXPR std::uint32_t philox_weyl_0 = 0x9E3779B9u;
REXLIB_CONST_CONSTEXPR std::uint32_t philox_weyl_1 = 0xBB67AE85u;

inline
void philox_round(
	philox_counter_type &counter,
	const philox_key_type &key
) noexcept
{
	const auto product_0 =
		static_cast<std::uint64_t>(philox_multiplier_0) * counter[0];
	const auto product_1 =
		static_cast<std::uint64_t>(philox_multiplier_1) * counter[2];

	const auto high_0 = static_cast<std::uint32_t>(product_0 >> 32);
	const auto low_0 = static_cast<std::uint32_t>(product_0);
	const auto high_1 = static_cast<std::uint32_t>(product_1 >> 32);
	const auto low_1 = static_cast<std::uint32_t>(product_1);

	counter = {{
		high_1 ^ counter[1] ^ key[0],
		low_1,
		high_0 ^ counter[3] ^ key[1],
		low_0
	}};
}

} // namespace detail

inline
philox_counter_type philox4x32_10(
	philox_counter_type counter,
	philox_key_type key
) noexcept
{
	detail::philox_round(counter, key);
	for (int round = 1; round < 10; ++round)
	{
		key[0] += detail::philox_weyl_0;
		key[1] += detail::philox_weyl_1;
		detail::philox_round(counter, key);
	}
	return counter;
}

} // namespace cpu
} // namespace rexlib
