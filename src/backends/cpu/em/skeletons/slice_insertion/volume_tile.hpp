// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/math/fourier/frequency_interval.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A block of the stored rows of the half spectrum of a volume.
 *
 * The block spans a range of stored indices along z and along y, and every
 * stored index along x. A stored index holds the frequency equal to it up
 * to half the extent, rounded down, and that minus the extent beyond; with
 * an even extent, the index at half the extent holds both the frequency
 * half the extent and its negative.
 */
class volume_tile
{
public:
	/**
	 * @brief The most intervals of frequencies a range of stored indices
	 * maps to.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t max_interval_count = 2;

	using interval_list_type =
		std::array<frequency_interval, max_interval_count>;

	/**
	 * @brief Construct a block.
	 *
	 * @param extent The extent of the volume along y and z.
	 * @param z_begin The first stored index along z.
	 * @param z_end One past the last stored index along z.
	 * @param y_begin The first stored index along y.
	 * @param y_end One past the last stored index along y.
	 */
	volume_tile(
		std::size_t extent,
		std::size_t z_begin,
		std::size_t z_end,
		std::size_t y_begin,
		std::size_t y_end
	) noexcept;

	std::size_t get_extent() const noexcept;
	std::size_t get_z_begin() const noexcept;
	std::size_t get_z_end() const noexcept;
	std::size_t get_y_begin() const noexcept;
	std::size_t get_y_end() const noexcept;

	/**
	 * @brief List the frequencies the stored indices along z hold.
	 *
	 * @param intervals Receives the intervals, disjoint and ascending.
	 * @return std::size_t The number of intervals, 1 or 2.
	 */
	std::size_t list_z_frequencies(
		interval_list_type &intervals
	) const noexcept;

	/**
	 * @brief List the frequencies the stored indices along y hold.
	 *
	 * @param intervals Receives the intervals, disjoint and ascending.
	 * @return std::size_t The number of intervals, 1 or 2.
	 */
	std::size_t list_y_frequencies(
		interval_list_type &intervals
	) const noexcept;

private:
	std::size_t m_extent;
	std::size_t m_z_begin;
	std::size_t m_z_end;
	std::size_t m_y_begin;
	std::size_t m_y_end;
};

/**
 * @brief List the frequencies a range of stored indices of an axis holds.
 *
 * @param extent The extent of the axis.
 * @param begin The first stored index.
 * @param end One past the last stored index. Must exceed @p begin and not
 * exceed @p extent.
 * @param intervals Receives the intervals, disjoint and ascending.
 * @return std::size_t The number of intervals, 1 or 2.
 */
std::size_t list_stored_frequencies(
	std::size_t extent,
	std::size_t begin,
	std::size_t end,
	volume_tile::interval_list_type &intervals
) noexcept;

} // namespace cpu
} // namespace rexlib
