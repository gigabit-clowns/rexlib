// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_reader.hpp>

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstdint>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief An image_reader backed by a vector instead of by a file.
 *
 * Stands in for a format so that the interface can be exercised before one
 * exists. It holds a contiguous file of sixteen bit integers, which is
 * the encoding the conversion rule matters most for, and serves regions of
 * it into destinations of int16 or float32.
 */
class fake_image_reader final
	: public image_reader
{
public:
	fake_image_reader(
		span<const std::size_t> extents,
		image_metadata metadata
	);

	~fake_image_reader() override;

	span<const std::size_t> get_extents() const noexcept override;

	numerical_type get_data_type() const noexcept override;
	const image_metadata& get_metadata() const noexcept override;

	void read(
		array_ref destination,
		const image_transfer_plan &regions
	) const override;

	/**
	 * @brief Get the value the file holds at a linear position.
	 */
	std::int16_t get_element(std::size_t index) const;

private:
	std::vector<std::size_t> m_extents;
	std::vector<std::int16_t> m_data;
	image_metadata m_metadata;
};

} // namespace em
} // namespace rexlib
