// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_transfer_plan.hpp>

namespace rexlib
{

class const_array_ref;

namespace em
{

class image_descriptor;

/**
 * @brief Abstract writable view of one image file.
 *
 * The mirror of @ref image_reader: the same regions, described by the same
 * @ref image_transfer_plan, the same conversion rule, the opposite direction.
 * A writer is opened over complete extents, so the shape of the file is
 * settled before anything is written; the file can be laid out once up
 * front and a region can be written wherever it belongs, in any order.
 * Every exception a writer throws names its file.
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
	 * @brief Get the shape and data type of the file.
	 *
	 * The ones the writer was created over. The extents bound every region
	 * that may be written, and a write converts to the data type from
	 * whatever it is given. How the file lays its elements out within the
	 * extents is the format's own business and is not reported.
	 *
	 * @return const image_descriptor& The descriptor. It refers to storage
	 * owned by this writer.
	 */
	virtual const image_descriptor& get_descriptor() const noexcept = 0;

	/**
	 * @brief Write a set of hyperrectangles of the file from one array.
	 *
	 * The mirror of @ref image_reader::read, sharing its plan and naming its
	 * sides the same way: the file is the file side and @p source the array
	 * side, so every region names where it is taken from in @p source and
	 * where it lands in the file, and all of them share the extents the plan
	 * carries. The plan is therefore built the same way for a write as for a
	 * read. Passing every region in one call lets a writer order and merge
	 * the accesses it is about to make, and pays for the source's geometry
	 * once.
	 *
	 * @p source is the array as the caller holds it and may be strided. Its
	 * rank must be the array rank of the plan and the file's the file rank,
	 * and values are converted with @ref numerical_cast semantics without
	 * scaling or normalising.
	 *
	 * Regions that do not overlap may be written by concurrent calls.
	 * Overlapping ones may not, and where two regions of one call land on the
	 * same elements of the file, which one prevails is unspecified.
	 *
	 * A region that is never written keeps whatever the format leaves in a
	 * file it has laid out but not filled. An empty plan writes nothing
	 * and succeeds.
	 *
	 * @param source The values to write. Must be initialized and host
	 * accessible.
	 * @param regions The regions to write and where each one comes from.
	 * @throws std::invalid_argument If the array offsets do not have the
	 * rank of @p source, if the file offsets do not have the rank of
	 * the file, or if @p source is not initialized.
	 * @throws std::out_of_range If a region is not contained in the file,
	 * or is not contained in @p source where it is taken from.
	 * @throws unsupported_operation_error If the data type of the file can
	 * not be produced from the one of @p source.
	 * @throws unsupported_capability_error If @p source is not host
	 * accessible.
	 * @throws image_file_error If the file can not be written.
	 */
	virtual void write(
		const_array_ref source,
		const image_transfer_plan &regions
	) = 0;

	/**
	 * @brief Make everything written so far reach the storage.
	 *
	 * Call this before dropping a writer whose failures matter. A writer
	 * flushes itself when it is destroyed, but a destructor can not report
	 * a failure, so a write that only fails on the way out is lost unless it
	 * was flushed explicitly.
	 *
	 * @throws image_file_error If the pending writes could not be
	 * completed.
	 */
	virtual void flush() = 0;
};

} // namespace em
} // namespace rexlib
