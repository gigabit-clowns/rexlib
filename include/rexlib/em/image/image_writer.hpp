// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>

namespace rexlib
{

class const_array_ref;

namespace em
{

/**
 * @brief Abstract writable view of one image file.
 *
 * The mirror of @ref image_reader: the same regions, described by the same
 * @ref image_transfer_plan, the same conversion rule, the opposite direction.
 * A writer is opened over complete extents, so the shape of the file is
 * settled before anything is written; the file can be laid out once up
 * front and a region can be written wherever it belongs, in any order.
 *
 * Growing a file whose length is not known until the data runs out is a
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
	 * @brief Get the extents of the file.
	 *
	 * The ones the writer was created over, which is what bounds every
	 * region that may be written. How the file lays its elements out within
	 * them is the format's own business and is not reported.
	 *
	 * @return span<const std::size_t> The extents, slowest axis first. It
	 * refers to storage owned by this writer.
	 */
	virtual span<const std::size_t> get_extents() const noexcept = 0;

	/**
	 * @brief Get how many of the extents describe one image or volume.
	 *
	 * That many trailing extents are one image or one volume, and the
	 * leading ones are the axes the file stacks along. It is therefore the
	 * dimensionality of what the file holds, two for an image and three for
	 * a volume, and it is what tells a stack of @c N images from one volume
	 * of @c N planes: their extents are identical and only this differs.
	 * Each format records the difference in its own way, as MRC does
	 * through its space group and its grid size.
	 *
	 * Equal to the rank of @ref get_extents for a file holding a single
	 * image or volume.
	 *
	 * @return std::size_t The rank of one image or volume. Never zero, and
	 * never above the rank of @ref get_extents.
	 */
	virtual std::size_t get_core_rank() const noexcept = 0;

	/**
	 * @brief Get the data type of the elements of the file.
	 *
	 * The one the writer was created over. A write converts to it from
	 * whatever it is given.
	 *
	 * @return numerical_type The data type.
	 */
	virtual numerical_type get_data_type() const noexcept = 0;

	/**
	 * @brief Write a set of hyperrectangles of the file from one array.
	 *
	 * The mirror of @ref image_reader::read, sharing its plan with the two
	 * sides the other way round: @p source is the source of the plan and
	 * the file its destination, so every region names where it is taken from
	 * in @p source and where it lands in the file, and all of them share the
	 * extents the plan carries. Passing
	 * every region in one call lets a writer order and merge the accesses
	 * it is about to make, and pays for the source's geometry once.
	 *
	 * @p source is the array as the caller holds it and may be strided. Its
	 * rank must be the source rank of the plan and the file's the
	 * destination rank, and values are converted with @ref numerical_cast
	 * semantics without scaling or normalising.
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
	 * @throws std::invalid_argument If the source offsets do not have the
	 * rank of @p source, if the destination offsets do not have the rank of
	 * the file, or if @p source is not initialized.
	 * @throws std::out_of_range If a region is not contained in the file,
	 * or is not contained in @p source where it is taken from.
	 * @throws invalid_operation_error If the data type of the file can not
	 * be produced from the one of @p source, or if @p source is not host
	 * accessible.
	 * @throws image_format_error If the file can not be written.
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
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	virtual void flush() = 0;
};

} // namespace em
} // namespace rexlib
