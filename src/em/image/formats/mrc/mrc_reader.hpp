// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mrc_geometry.hpp"
#include "mrc_header.hpp"

#include <em/image/memory_mapping/image_file_mapping.hpp>

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <string>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief One MRC file opened for reading.
 *
 * The file is mapped when the reader is opened and its header parsed once,
 * so reading a region is the transfer and nothing else. Everything the reader
 * reports is settled at that point and reading does not change it, which is
 * what lets @ref read be called concurrently: the mapping and the layout it
 * builds per call are the only state a read touches, and neither is shared
 * across calls.
 *
 * A read asks for the stretches it is about to touch before it touches them,
 * a step ahead of the one it is walking. See @ref image_region_prefetch_plan,
 * which works out what those stretches and steps are.
 */
class mrc_reader final
	: public image_reader
{
public:
	/**
	 * @brief Open a file for reading.
	 *
	 * @param path Path to the file.
	 * @throws image_format_error If the file can not be mapped, if its
	 * header is malformed, or if it is shorter than that header says it is.
	 */
	explicit mrc_reader(const std::string &path);

	~mrc_reader() override = default;

	span<const std::size_t> get_extents() const noexcept override;

	std::size_t get_core_rank() const noexcept override;

	numerical_type get_data_type() const noexcept override;

	const image_metadata& get_metadata() const noexcept override;

	void read(
		array_ref destination,
		const image_transfer_plan &regions
	) const override;

private:
	image_file_mapping m_mapping;
	mrc_header m_header;
	mrc_geometry m_geometry;
	image_metadata m_metadata;
};

} // namespace mrc
} // namespace em
} // namespace rexlib
