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

inline
std::uint64_t join_words(std::uint32_t low, std::uint32_t high) noexcept
{
	return (static_cast<std::uint64_t>(high) << 32) | low;
}

inline
std::uint32_t low_word(std::uint64_t value) noexcept
{
	return static_cast<std::uint32_t>(value);
}

inline
std::uint32_t high_word(std::uint64_t value) noexcept
{
	return static_cast<std::uint32_t>(value >> 32);
}

} // namespace detail

inline
double uniform_open_closed(std::uint64_t bits) noexcept
{
	return static_cast<double>((bits >> 11) + 1) *
		detail::inverse_two_to_53;
}

inline
std::array<double, 2> standard_normal_pair(
	std::uint64_t seed,
	std::uint64_t block
) noexcept
{
	const auto words = philox4x32_10(
		{{ detail::low_word(block), detail::high_word(block), 0, 0 }},
		{{ detail::low_word(seed), detail::high_word(seed) }}
	);

	const auto radius = std::sqrt(
		-2.0 * std::log(
			uniform_open_closed(detail::join_words(words[0], words[1]))
		)
	);
	const auto angle = detail::two_pi *
		uniform_open_closed(detail::join_words(words[2], words[3]));

	return {{ radius * std::cos(angle), radius * std::sin(angle) }};
}

inline
double standard_normal(std::uint64_t seed, std::uint64_t counter) noexcept
{
	return standard_normal_pair(seed, counter / 2)[counter % 2];
}

} // namespace cpu
} // namespace rexlib
