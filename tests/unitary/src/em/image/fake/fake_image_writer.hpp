// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_writer.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstdint>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief An image_writer backed by a vector instead of by a file.
 *
 * The mirror of @ref fake_image_reader: it lays out a file of sixteen bit
 * integers when it is constructed and takes regions of int16 or float32
 * into it.
 */
class fake_image_writer final
	: public image_writer
{
public:
	fake_image_writer(
		span<const std::size_t> extents,
		std::size_t core_rank
	);

	~fake_image_writer() override;

	span<const std::size_t> get_extents() const noexcept override;

	numerical_type get_data_type() const noexcept override;

	std::size_t get_core_rank() const noexcept override;

	void write(
		const_array_ref source,
		const image_transfer_plan &regions
	) override;

	void flush() override;

	/**
	 * @brief Get the value the file holds at a linear position.
	 */
	std::int16_t get_element(std::size_t index) const;

	/**
	 * @brief Get how many times the writer has been flushed.
	 */
	std::size_t get_flush_count() const noexcept;

private:
	std::vector<std::size_t> m_extents;
	std::size_t m_core_rank;
	std::vector<std::int16_t> m_data;
	std::size_t m_flush_count;
};

} // namespace em
} // namespace rexlib
