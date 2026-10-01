// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_file_layout.hpp"
#include "image_file_mapping.hpp"
#include "image_prefetch_schedule.hpp"

#include <em/image/formats/strided_transfer/image_region_read_walk.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief Reads a file whose values are laid out as an @ref image_file_layout
 * states, through a mapping of it.
 *
 * The file is mapped when the reader is constructed, so reading a region is
 * the transfer and nothing else. Everything the reader reports is settled at
 * that point and reading does not change it, which is what lets @ref read be
 * called concurrently: the mapping and what a call resolves are the only
 * state a read touches, and the latter is not shared across calls.
 *
 * A read asks for the stretches it is about to touch before it touches them,
 * a step ahead of the one it is walking. See @ref image_prefetch_schedule,
 * which works out what those stretches and steps are.
 */
class mapped_image_reader final
	: public image_reader
{
public:
	/**
	 * @brief Map a file for reading.
	 *
	 * @param path Path to the file.
	 * @param layout Where and how the file holds its values.
	 * @param metadata What the file states beyond its shape and data type.
	 * @throws image_file_error If the file can not be mapped.
	 * @throws image_format_error If the file is shorter than @p layout says
	 * it is.
	 */
	mapped_image_reader(
		const std::string &path,
		image_file_layout layout,
		image_metadata metadata
	);

	~mapped_image_reader() override = default;

	const image_descriptor& get_descriptor() const noexcept override;

	const image_metadata& get_metadata() const noexcept override;

	void read(
		array_ref destination,
		const image_transfer_plan &regions
	) const override;

private:
	/**
	 * @brief Resolve the regions of a plan between the file and an array.
	 *
	 * @param destination The array the regions land in.
	 * @param regions The regions to read.
	 * @return image_region_read_walk The regions, bounds checked and ready
	 * to be walked.
	 */
	image_region_read_walk resolve_regions(
		array_ref destination,
		const image_transfer_plan &regions
	) const;

	/**
	 * @brief Work out the stretches of the mapping resolved regions reach,
	 * and the steps they are asked for in.
	 *
	 * @param regions The regions to read.
	 * @param walk What @ref resolve_regions made of them.
	 * @return image_prefetch_schedule The stretches and their steps.
	 */
	image_prefetch_schedule schedule_prefetch(
		const image_transfer_plan &regions,
		const image_region_read_walk &walk
	) const;

	/**
	 * @brief Move the regions a step at a time, asking for each step before
	 * the one ahead of it is walked.
	 *
	 * @param walk The resolved regions.
	 * @param schedule The steps to move them in.
	 * @param array_data First element of the array.
	 * @param array_type Data type of the array.
	 */
	void move_regions(
		const image_region_read_walk &walk,
		const image_prefetch_schedule &schedule,
		void *array_data,
		numerical_type array_type
	) const;

	image_file_mapping m_mapping;
	image_file_layout m_layout;
	image_metadata m_metadata;
};

} // namespace em
} // namespace rexlib
