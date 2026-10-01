// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/span.hpp>

#include <memory>
#include <string>

namespace rexlib
{

class completion;
class execution_context;

namespace em
{

class image_location;
class image_reader_provider;
class image_source;
class index_table;

/**
 * @brief Read a whole file into an array of its own.
 *
 * Gets a reader over the file from @p readers, allocates what the file holds
 * and fills it, all before returning.
 *
 * @param path Path to the file to read.
 * @param readers Where the file becomes a reader.
 * @param context Where the array is allocated.
 * @param data_type Data type of the array, the values of the file being
 * converted to it, or unknown to keep the one the file holds.
 * @return array The contents of the file.
 * @throws unsupported_operation_error If @p data_type can not be produced
 * from the one of the file.
 *
 * @see read_batch_async
 */
REXLIB_API
array read(
	const std::string &path,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type = numerical_type::unknown
);

/**
 * @brief Read a whole file, or one image or volume of a stack, into an array
 * of its own.
 *
 * @param location The file, or the image or volume of it, to read.
 * @param readers Where the file becomes a reader.
 * @param context Where the array is allocated.
 * @param data_type Data type of the array, the values of the file being
 * converted to it, or unknown to keep the one the file holds.
 * @return array The contents of what @p location names.
 * @throws unsupported_operation_error If @p data_type can not be produced
 * from the one of the file.
 */
REXLIB_API
array read(
	const image_location &location,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type = numerical_type::unknown
);

/**
 * @brief Read one image or volume per location into a batch,
 * asynchronously.
 *
 * Each slot of @p destination receives what its location names: the image
 * or volume @ref image_location::get_index_in_stack indexes within its file,
 * or the whole file when the location carries no index. A batch may not mix
 * the two; every location must either carry an index in a stack or none may.
 * The reads are handed to @p source as one transaction.
 *
 * Returns before the reads are done, unlike @ref read, and fills an array
 * the caller already has rather than allocating one.
 *
 * Every slot must be there to read: a location naming an index its stack
 * does not hold is reported through the completion rather than skipped. See
 * @ref strict_image_transfer_sanitizer.
 *
 * @param source Where the reads are dispatched. Needs to outlive this call
 * and no longer, the work outliving it carrying what it needs.
 * @param destination Where the images or volumes land. Its leading extent
 * is the batch size and its remaining extents are the shape of one image or
 * volume.
 * @param locations Where each slot comes from, one per slot of
 * @p destination and in the same order.
 * @return std::shared_ptr<completion> The completion, never null.
 * @throws std::invalid_argument If @p destination has no extents, if its
 * leading extent is not the number of locations, or if @p locations mixes
 * those carrying an index in a stack with those carrying none.
 */
REXLIB_API
std::shared_ptr<completion> read_batch_async(
	const image_source &source,
	array destination,
	span<const image_location> locations
);

/**
 * @brief Crop a batch of equally sized patches out of one image,
 * asynchronously.
 *
 * Every patch comes from the image @p location names, which is either the
 * image @ref image_location::get_index_in_stack indexes within the file or
 * the whole file when it carries no index in a stack.
 *
 * @par Centres
 * A patch spans from @c centre-extent/2 along each axis, so its centre lands
 * at index @c extent/2 within it. That is the middle sample for an odd extent
 * and the origin a Fourier transform of the patch would use for an even one.
 *
 * @par Borders
 * A patch reaching past the edge of the image is read as far as the image
 * goes and no further. The elements of @p destination no data reached are
 * left untouched, so a patch is padded with whatever @p destination held
 * beforehand. Filling it first with a value that cannot occur in the image,
 * such as a quiet NaN, marks the padding. See
 * @ref clipping_image_transfer_sanitizer.
 *
 * @param source Where the reads are dispatched. Needs to outlive this call
 * and no longer.
 * @param destination Where the patches land. Its leading extent is the batch
 * size and its remaining extents are the shape of one patch.
 * @param location The image every patch is cropped from.
 * @param centres Centre of each patch, of the rank of one patch and as many
 * as the batch size.
 * @return std::shared_ptr<completion> The completion, never null.
 * @throws std::invalid_argument If @p destination has no extents, if
 * @p centres does not hold one centre per slot of @p destination, or if the
 * centres do not have the rank of one patch.
 */
REXLIB_API
std::shared_ptr<completion> read_patches_async(
	const image_source &source,
	array destination,
	const image_location &location,
	const index_table &centres
);

} // namespace em
} // namespace rexlib
