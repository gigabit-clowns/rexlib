// SPDX-License-Identifier: GPL-3.0-only

#include "standard_normal.hpp"

#include "philox.hpp"

#include <rexlib/core/platform/constexpr.hpp>

#include <cmath>

namespace rexlib
{
namespace cpu
{

namespace detail
{

REXLIB_CONST_CONSTEXPR double inverse_two_to_53 = 1.0 / 9007199254740992.0;
REXLIB_CONST_CONSTEXPR double two_pi = 6.283185307179586476925;

} // namespace detail

inline
double uniform_open_closed(std::uint64_t bits) noexcept
{
	return static_cast<double>((bits >> 11) + 1) *
		detail::inverse_two_to_53;
}

inline
std::array<double, 2> standard_normal_pair(
	std::uint64_t key,
	std::uint64_t block
) noexcept
{
	const auto halves = make_philox_halves(
		philox4x32_10(
			make_philox_counter(block, 0, philox_domain::draw),
			make_philox_key(key)
		)
	);

	const auto radius =
		std::sqrt(-2.0 * std::log(uniform_open_closed(halves[0])));
	const auto angle = detail::two_pi * uniform_open_closed(halves[1]);

	return {{ radius * std::cos(angle), radius * std::sin(angle) }};
}

inline
double standard_normal(std::uint64_t key, std::uint64_t counter) noexcept
{
	return standard_normal_pair(key, counter / 2)[counter % 2];
}

} // namespace cpu
} // namespace rexlib
