// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief The regions of a plan, resolved into those of each page they
 * reach.
 *
 * A TIFF file holds its pages one by one, so regions stated against the
 * whole of it are moved a page at a time. Each page a plan reaches gets the
 * regions that lie on it, stated against that page alone: as offsets into
 * its rows and columns, with the page axis gone. A region that spans
 * several pages becomes one region on each of them, placed one position
 * further along the axis of the array its pages run along.
 *
 * The pages are held in ascending order, and each carries the rows its
 * regions reach, which are all of it that needs decoding.
 */
class tiff_page_regions
{
public:
	/**
	 * @brief Resolve the regions of a plan into those of each page.
	 *
	 * @param regions The regions, stated against a file of rank two, which
	 * is a single page, or of rank three, whose first axis runs along its
	 * pages. They must be contained in the file.
	 * @throws std::invalid_argument If the file rank of @p regions is
	 * neither two nor three.
	 */
	explicit tiff_page_regions(const image_transfer_plan &regions);

	tiff_page_regions(const tiff_page_regions &other) = default;
	tiff_page_regions(tiff_page_regions &&other) noexcept = default;
	~tiff_page_regions() = default;

	tiff_page_regions& operator=(const tiff_page_regions &other) = default;
	tiff_page_regions&
	operator=(tiff_page_regions &&other) noexcept = default;

	/**
	 * @brief Get how many pages the regions reach.
	 *
	 * @return std::size_t The number of pages.
	 */
	std::size_t get_page_count() const noexcept;

	/**
	 * @brief Get the index of one of the pages reached.
	 *
	 * @param position Position among the pages reached. Must be below
	 * @ref get_page_count.
	 * @return std::size_t The index of the page in the file, which ascends
	 * with @p position.
	 */
	std::size_t get_page(std::size_t position) const noexcept;

	/**
	 * @brief Get the regions that lie on one of the pages reached.
	 *
	 * @param position Position among the pages reached. Must be below
	 * @ref get_page_count.
	 * @return const image_transfer_plan& The regions, stated against a file
	 * of rank two and the array of the plan they were resolved from. It
	 * refers to storage owned by this object.
	 */
	const image_transfer_plan&
	get_regions(std::size_t position) const noexcept;

	/**
	 * @brief Get the first row the regions of a page reach.
	 *
	 * @param position Position among the pages reached. Must be below
	 * @ref get_page_count.
	 * @return std::size_t The index of the row.
	 */
	std::size_t get_first_row(std::size_t position) const noexcept;

	/**
	 * @brief Get how many rows the regions of a page reach.
	 *
	 * @param position Position among the pages reached. Must be below
	 * @ref get_page_count.
	 * @return std::size_t The number of rows from @ref get_first_row to the
	 * last one any of the regions reaches.
	 */
	std::size_t get_row_count(std::size_t position) const noexcept;

private:
	std::vector<std::size_t> m_pages;
	std::vector<image_transfer_plan> m_regions;
	std::vector<std::size_t> m_first_rows;
	std::vector<std::size_t> m_end_rows;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
