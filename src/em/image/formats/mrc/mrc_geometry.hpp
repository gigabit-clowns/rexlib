// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mrc_header.hpp"
#include "mrc_single_section.hpp"

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief What a header says about the shape of the values of a file.
 *
 * An MRC file states its shape as three counts and a space group, and the
 * same three counts mean different things depending on that space group: a
 * stack of images and one volume of the same depth differ only in it, and a
 * stack of volumes divides the sections between its two leading axes, which
 * MRC2014 states by the space group alone, even for a stack of one volume or
 * of volumes one section deep. The one case the standard leaves open, a
 * single section in the image space group, is either one image or a stack of
 * one, and is settled by the caller. This resolves all of it once, when a
 * file is opened, into the descriptor an @ref image_reader reports.
 *
 * The values themselves are laid out with the columns changing fastest, and
 * the header names the axis of space the columns, the rows and the sections
 * each run along. This reports them along the axes of space, so a file that
 * names them in another order is reported with the strides of its axes out of
 * descending order rather than with its axes transposed. One that names no
 * axis at all is read as though it named them in order.
 */
class mrc_geometry
{
public:
	/**
	 * @brief Derive the shape of a file from its header.
	 *
	 * @param header The header of the file.
	 * @param single_section What the file holds when the header states a
	 * single section in the image space group.
	 * @throws image_format_error If the axis correspondence of the header
	 * names anything but the three axes of space, one each, without being
	 * unset, or if the values of the file would not begin at an offset its
	 * elements can be addressed at.
	 */
	explicit mrc_geometry(
		const mrc_header &header,
		mrc_single_section single_section = mrc_single_section::image
	);

	mrc_geometry(const mrc_geometry &other) = default;
	mrc_geometry(mrc_geometry &&other) noexcept = default;
	~mrc_geometry() = default;

	mrc_geometry& operator=(const mrc_geometry &other) = default;
	mrc_geometry& operator=(mrc_geometry &&other) noexcept = default;

	/**
	 * @brief Get the shape and data type of the values of the file.
	 *
	 * The axes of a stack come first, and the axes of one image or volume
	 * follow in the order of the axes of space, the one along the first axis
	 * of space last. The core rank is two for a file of images and three for
	 * one of volumes.
	 *
	 * @return const image_descriptor& The descriptor.
	 */
	const image_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the distance between consecutive elements along each axis.
	 *
	 * In elements rather than bytes, and of the same rank as the extents and
	 * in their order.
	 *
	 * @return span<const std::ptrdiff_t> The strides.
	 */
	span<const std::ptrdiff_t> get_strides() const noexcept;

	/**
	 * @brief Get where the values of the file begin.
	 *
	 * @return std::size_t The offset in bytes.
	 */
	std::size_t get_data_offset() const noexcept;

	/**
	 * @brief Get how many elements the file holds.
	 *
	 * @return std::size_t The product of the extents.
	 */
	std::size_t get_element_count() const noexcept;

	/**
	 * @brief Get how many bytes the values of the file occupy.
	 *
	 * @return std::size_t The size in bytes, past @ref get_data_offset.
	 */
	std::size_t get_data_size() const noexcept;

private:
	image_descriptor m_descriptor;
	std::vector<std::ptrdiff_t> m_strides;
	std::size_t m_data_offset;

	mrc_geometry(
		const mrc_header &header,
		mrc_single_section single_section,
		const std::vector<std::size_t> &axis_order
	);
};

} // namespace mrc
} // namespace em
} // namespace rexlib
