// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_region_list.hpp>

#include <cstddef>

namespace rexlib
{

class array_descriptor;
class const_array_ref;

namespace em
{

/**
 * @brief Abstract writable view of a single rectangular ND image dataset.
 *
 * The mirror of @ref image_reader: the same regions, described by the same
 * @ref image_region_list, the same conversion rule, the opposite direction.
 * A writer is opened over a complete descriptor, so the extents of the
 * dataset are settled before anything is written; the file can be laid out
 * once up front and a region can be written wherever it belongs, in any
 * order.
 *
 * Growing a dataset whose length is not known until the data runs out is a
 * different contract, needing a cursor and a count patched as it goes, and
 * is not this interface.
 */
class REXLIB_API image_writer
{
public:
	image_writer() noexcept;
	image_writer(const image_writer &other) = delete;
	image_writer(image_writer &&other) = delete;
	virtual ~image_writer();

	image_writer& operator=(const image_writer &other) = delete;
	image_writer& operator=(image_writer &&other) = delete;

	/**
	 * @brief Get the descriptor of the whole dataset.
	 *
	 * The one the writer was opened with, which is what bounds every region
	 * that may be written.
	 *
	 * @return const array_descriptor& The descriptor.
	 */
	virtual const array_descriptor& get_descriptor() const noexcept = 0;

	/**
	 * @brief Write a set of hyperrectangles of the dataset from one array.
	 *
	 * The mirror of @ref image_reader::read, sharing its list: every region
	 * names where it lands in the dataset and where it is taken from in
	 * @p source, and all of them share the extents the list carries. Passing
	 * the whole batch in one call lets a writer order and merge the accesses
	 * it is about to make, and pays for the source's geometry once.
	 *
	 * @p source is the array as the caller holds it and may be strided. As in
	 * a read the rank is never adjusted implicitly, and values are converted
	 * with @ref numerical_cast semantics without scaling or normalising.
	 *
	 * Regions that do not overlap may be written by concurrent calls.
	 * Overlapping ones may not, and where two regions of one call land on the
	 * same elements of the dataset, which one prevails is unspecified.
	 *
	 * A region that is never written keeps whatever the format leaves in a
	 * dataset it has laid out but not filled. An empty list writes nothing
	 * and succeeds.
	 *
	 * @param source The values to write. Must be initialized and host
	 * accessible.
	 * @param regions The regions to write and where each one comes from.
	 * @throws std::invalid_argument If the rank of the extents does not match
	 * the rank of @p source, if the rank of the dataset offsets does not
	 * match the rank of the dataset, or if @p source is not initialized.
	 * @throws std::out_of_range If a region is not contained in the dataset,
	 * or is not contained in @p source where it is taken from.
	 * @throws invalid_operation_error If the data type of the dataset can not
	 * be produced from the one of @p source, or if @p source is not host
	 * accessible.
	 * @throws image_format_error If the dataset can not be written.
	 */
	virtual void write(
		const_array_ref source,
		const image_region_list &regions
	) = 0;

	/**
	 * @brief Make everything written so far reach the storage.
	 *
	 * Call this before dropping a writer whose failures matter. A writer
	 * flushes itself when it is destroyed, but a destructor can not report
	 * a failure, so a write that only fails on the way out is lost unless it
	 * was flushed explicitly.
	 *
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	virtual void flush() = 0;
};

} // namespace em
} // namespace rexlib
