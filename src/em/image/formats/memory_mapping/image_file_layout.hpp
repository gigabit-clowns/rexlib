// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief Where and how a file holds its values.
 *
 * What the file holds, the distance between consecutive elements along each
 * of its axes, the byte its values begin at and the byte order they are
 * stated in. It is what a format resolves the header of a file into, and all
 * that moving the values of the file needs to know about its format.
 *
 * The strides are in elements rather than bytes and need not be descending:
 * a file that stores its axes in another order than it reports them states
 * so through them.
 */
class image_file_layout
{
public:
	/**
	 * @brief Construct a layout from its components.
	 *
	 * @param descriptor What the file holds.
	 * @param strides Distance between consecutive elements along each axis,
	 * in elements and in the order of the extents of @p descriptor.
	 * @param data_offset Where the values begin, in bytes from the start of
	 * the file.
	 * @param order Byte order the values are stated in.
	 * @throws std::invalid_argument If @p strides do not have the rank of
	 * the extents of @p descriptor.
	 * @throws image_format_error If @p data_offset is not a multiple of the
	 * size of one element, where its values could not be addressed.
	 */
	image_file_layout(
		image_descriptor descriptor,
		std::vector<std::ptrdiff_t> strides,
		std::size_t data_offset,
		byte_order order
	);

	image_file_layout(const image_file_layout &other) = default;
	image_file_layout(image_file_layout &&other) noexcept = default;
	~image_file_layout() = default;

	image_file_layout& operator=(const image_file_layout &other) = default;
	image_file_layout&
	operator=(image_file_layout &&other) noexcept = default;

	/**
	 * @brief Get the shape and data type of the values of the file.
	 *
	 * @return const image_descriptor& The descriptor.
	 */
	const image_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the distance between consecutive elements along each axis.
	 *
	 * @return span<const std::ptrdiff_t> The strides, in elements and in the
	 * order of the extents.
	 */
	span<const std::ptrdiff_t> get_strides() const noexcept;

	/**
	 * @brief Get where the values of the file begin.
	 *
	 * @return std::size_t The offset in bytes.
	 */
	std::size_t get_data_offset() const noexcept;

	/**
	 * @brief Get how many bytes the values of the file occupy.
	 *
	 * @return std::size_t The size in bytes, past @ref get_data_offset.
	 */
	std::size_t get_data_size() const noexcept;

	/**
	 * @brief Get the byte order the values are stated in.
	 *
	 * @return byte_order The byte order.
	 */
	byte_order get_byte_order() const noexcept;

private:
	image_descriptor m_descriptor;
	std::vector<std::ptrdiff_t> m_strides;
	std::size_t m_data_offset;
	byte_order m_byte_order;
};

} // namespace em
} // namespace rexlib
