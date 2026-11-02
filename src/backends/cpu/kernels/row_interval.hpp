// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "frequency_bounds.hpp"
#include "slice_frame.hpp"

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A half-open range of the samples of a row of a slice.
 */
class row_interval
{
public:
	/**
	 * @brief Construct a range.
	 *
	 * @param begin The first sample.
	 * @param end One past the last sample. A range with end at or below
	 * begin is empty.
	 */
	row_interval(std::ptrdiff_t begin, std::ptrdiff_t end) noexcept;

	/**
	 * @brief Get the first sample.
	 *
	 * @return std::ptrdiff_t The sample.
	 */
	std::ptrdiff_t get_begin() const noexcept;

	/**
	 * @brief Get one past the last sample.
	 *
	 * @return std::ptrdiff_t The sample.
	 */
	std::ptrdiff_t get_end() const noexcept;

	/**
	 * @brief Check whether the range holds no sample.
	 *
	 * @return bool True when it is empty.
	 */
	bool is_empty() const noexcept;

private:
	std::ptrdiff_t m_begin;
	std::ptrdiff_t m_end;
};

/**
 * @brief Clip a row of a slice to the samples that may land in a box.
 *
 * The sample k of the row lands on base + k step. The range returned
 * holds every k in [first, last] whose frequency lies in the box, and may
 * hold one more at each end. A step of 0 along an axis keeps the whole row
 * when the base lies within the bounds of that axis, and nothing otherwise.
 * A base or step that is not finite gives an empty range.
 *
 * @param base The frequency of the sample k = 0.
 * @param step The step of the frequency per sample.
 * @param bounds The box.
 * @param first The first sample of the row.
 * @param last The last sample of the row.
 * @return row_interval The range.
 */
row_interval clip_row(
	const volume_frequency &base,
	const volume_frequency &step,
	const frequency_bounds &bounds,
	std::ptrdiff_t first,
	std::ptrdiff_t last
) noexcept;

/**
 * @brief Bound the rows of a slice whose samples may land in a box.
 *
 * The sample (k_x, k_y) lands on k_x x_step + k_y y_step. The range
 * returned holds every k_y in [first_y, last_y] for which some k_x in
 * [first_x, last_x] lands in the box, and may hold more. A step that is not
 * finite gives an empty range.
 *
 * @param x_step The step of the frequency per unit of k_x.
 * @param y_step The step of the frequency per unit of k_y.
 * @param bounds The box.
 * @param first_x The first sample of a row.
 * @param last_x The last sample of a row.
 * @param first_y The first row.
 * @param last_y The last row.
 * @return row_interval The range of rows.
 */
row_interval bound_rows(
	const volume_frequency &x_step,
	const volume_frequency &y_step,
	const frequency_bounds &bounds,
	std::ptrdiff_t first_x,
	std::ptrdiff_t last_x,
	std::ptrdiff_t first_y,
	std::ptrdiff_t last_y
) noexcept;

} // namespace cpu
} // namespace rexlib
