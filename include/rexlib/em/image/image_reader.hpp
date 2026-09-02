// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>

namespace rexlib
{

class array_descriptor;
class array_ref;

namespace em
{

class image_access_traits;
class image_metadata;

/**
 * @brief Abstract read-only view of one image file.
 *
 * A reader is opened over one image file and exposes its contents as one
 * rectangular ND array. Every
 * read is a set of hyperrectangles of that array, which is the only access
 * pattern there is: element @c i of an @c (N,H,W) stack is the region at
 * offset @c (i,0,0) with extents @c (1,H,W), a patch of an @c (H,W)
 * micrograph is the region at @c (y,x) with extents @c (h,w), and a whole
 * volume is the region covering everything. Reading in a random or in a
 * sequential order is a property of a sequence of calls rather than of one,
 * so it is stated through @ref image_open_options when the file is opened
 * and is nothing a reader distinguishes.
 *
 * A read takes a whole batch of regions rather than one at a time. That is
 * the only shape in which a reader can see enough to order and merge the
 * accesses it is about to make, and it keeps the per region cost to
 * arithmetic.
 *
 * A file holding several images of different shapes is opened once per
 * image, selected through @ref image_open_options::get_image_index.
 *
 * Everything a reader reports is fixed when it is opened, and reading does
 * not change it. What varies between formats is what a read costs, which
 * @ref get_access_traits states.
 */
class REXLIB_API image_reader
{
public:
	image_reader() noexcept;
	image_reader(const image_reader &other) = delete;
	image_reader(image_reader &&other) = delete;
	virtual ~image_reader();

	image_reader& operator=(const image_reader &other) = delete;
	image_reader& operator=(image_reader &&other) = delete;

	/**
	 * @brief Get the descriptor of the whole file.
	 *
	 * The extents are those of the file in the order it stores its axes,
	 * slowest first, and the data type is the one a read produces without
	 * converting.
	 *
	 * @return const array_descriptor& The descriptor.
	 */
	virtual const array_descriptor& get_descriptor() const noexcept = 0;

	/**
	 * @brief Get how the samples of the file map onto physical space.
	 *
	 * @return const image_metadata& The metadata. States nothing for a file
	 * that does not carry it.
	 */
	virtual const image_metadata& get_metadata() const noexcept = 0;

	/**
	 * @brief Get what a read from this reader costs.
	 *
	 * @return const image_access_traits& The traits.
	 */
	virtual const image_access_traits&
	get_access_traits() const noexcept = 0;

	/**
	 * @brief Read a set of hyperrectangles of the file into one array.
	 *
	 * The file is the source of the plan and @p destination its
	 * destination: every region names where it starts in the file and where
	 * it lands in @p destination, and all of them share the extents the
	 * plan carries. Passing every region in one call is what lets a
	 * reader sort the regions by their position on the storage and merge
	 * neighbouring ones into a single larger read, which one call per region
	 * can not express; it also pays for the destination's geometry once
	 * rather than once per region.
	 *
	 * @p destination is the array as the caller holds it, not a view of one
	 * slot, and it may be strided. The source offsets of the plan must have
	 * the rank of the file and its destination offsets the rank of
	 * @p destination; a side whose rank exceeds that of the extents spans a
	 * single position along the axes they do not reach.
	 *
	 * Regions may be read in any order. Where two of them land on the same
	 * elements of @p destination, which one prevails is unspecified.
	 *
	 * Values are converted to the data type of @p destination with
	 * @ref numerical_cast semantics, which preserve the numeric value.
	 * Nothing is scaled, normalised or clipped, whatever statistics the file
	 * may carry in its header; asking for
	 * @ref image_access_traits::get_preferred_data_type converts nothing at
	 * all.
	 *
	 * This method may be called concurrently on one reader. A reader that
	 * can not decode in parallel serialises the calls itself and leaves
	 * @ref image_access_flag_bits::concurrent_read clear, so a caller that
	 * ignores the flag loses throughput and never correctness.
	 *
	 * An empty plan reads nothing and succeeds.
	 *
	 * @param destination Where the regions are written. Must be initialized
	 * and host accessible.
	 * @param regions The regions to read and where each one lands.
	 * @throws std::invalid_argument If the source offsets do not have the
	 * rank of the file, if the destination offsets do not have the rank of
	 * @p destination, or if @p destination is not initialized.
	 * @throws std::out_of_range If a region is not contained in the file,
	 * or does not fit in @p destination where it is placed.
	 * @throws invalid_operation_error If the data type of @p destination can
	 * not be produced from the one of the file, or if @p destination is
	 * not host accessible.
	 * @throws image_format_error If the file turns out to be malformed
	 * or truncated where a region is read.
	 */
	virtual void read(
		array_ref destination,
		const image_transfer_plan &regions
	) const = 0;
};

} // namespace em
} // namespace rexlib
