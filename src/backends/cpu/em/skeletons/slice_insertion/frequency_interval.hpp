// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A closed interval of the signed frequencies of one axis of a
 * spectrum.
 */
class frequency_interval
{
public:
	/**
	 * @brief Construct the interval holding the frequency 0 alone.
	 */
	frequency_interval() noexcept;

	/**
	 * @brief Construct an interval.
	 *
	 * @param lower The lowest frequency.
	 * @param upper The highest frequency. Not below @p lower.
	 */
	frequency_interval(std::ptrdiff_t lower, std::ptrdiff_t upper) noexcept;

	/**
	 * @brief Get the lowest frequency.
	 *
	 * @return std::ptrdiff_t The frequency.
	 */
	std::ptrdiff_t get_lower() const noexcept;

	/**
	 * @brief Get the highest frequency.
	 *
	 * @return std::ptrdiff_t The frequency.
	 */
	std::ptrdiff_t get_upper() const noexcept;

	friend bool operator==(
		const frequency_interval &lhs,
		const frequency_interval &rhs
	) noexcept
	{
		return lhs.m_lower == rhs.m_lower && lhs.m_upper == rhs.m_upper;
	}

	friend bool operator!=(
		const frequency_interval &lhs,
		const frequency_interval &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	std::ptrdiff_t m_lower;
	std::ptrdiff_t m_upper;
};

} // namespace cpu
} // namespace rexlib
