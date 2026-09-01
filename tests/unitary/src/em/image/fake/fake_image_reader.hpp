// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_reader.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_access_traits.hpp>
#include <rexlib/em/image/image_metadata.hpp>

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
 * exists. It holds a contiguous dataset of sixteen bit integers, which is
 * the encoding the conversion rule matters most for, and serves regions of
 * it into destinations of int16 or float32.
 */
class fake_image_reader final
	: public image_reader
{
public:
	fake_image_reader(
		span<const std::size_t> extents,
		image_access_traits traits,
		image_metadata metadata
	);

	~fake_image_reader() override;

	const array_descriptor& get_descriptor() const noexcept override;
	const image_metadata& get_metadata() const noexcept override;
	const image_access_traits& get_access_traits() const noexcept override;

	void read_region(
		span<const std::size_t> offset,
		array_ref destination
	) const override;

	/**
	 * @brief Get the value the dataset holds at a linear position.
	 */
	std::int16_t get_element(std::size_t index) const;

private:
	std::vector<std::size_t> m_extents;
	std::vector<std::int16_t> m_data;
	array_descriptor m_descriptor;
	image_access_traits m_traits;
	image_metadata m_metadata;
};

} // namespace em
} // namespace rexlib
