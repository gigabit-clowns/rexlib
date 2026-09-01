// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_writer.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_region_list.hpp>

#include <cstdint>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief An image_writer backed by a vector instead of by a file.
 *
 * The mirror of @ref fake_image_reader: it lays out a dataset of sixteen bit
 * integers when it is constructed and takes regions of int16 or float32
 * into it.
 */
class fake_image_writer final
	: public image_writer
{
public:
	explicit fake_image_writer(span<const std::size_t> extents);

	~fake_image_writer() override;

	const array_descriptor& get_descriptor() const noexcept override;

	void write(
		const_array_ref source,
		const image_region_list &regions
	) override;

	void flush() override;

	/**
	 * @brief Get the value the dataset holds at a linear position.
	 */
	std::int16_t get_element(std::size_t index) const;

	/**
	 * @brief Get how many times the writer has been flushed.
	 */
	std::size_t get_flush_count() const noexcept;

private:
	std::vector<std::size_t> m_extents;
	std::vector<std::int16_t> m_data;
	array_descriptor m_descriptor;
	std::size_t m_flush_count;
};

} // namespace em
} // namespace rexlib
