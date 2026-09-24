// SPDX-License-Identifier: GPL-3.0-only

#pragma once

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
 * and fills it, all before returning. Its asynchronous peers below fill an
 * array the caller already has.
 *
 * @param path Path to the file to read.
 * @param readers Where the file becomes a reader.
 * @param context Where the array is allocated.
 * @return array The contents of the file, in its own data type.
 */
REXLIB_API
array read(
	const std::string &path,
	image_reader_provider &readers,
	const execution_context &context
);

/**
 * @brief Read a whole file, or one element of it, into an array of its own.
 *
 * @param location The file, or the element of it, to read.
 * @param readers Where the file becomes a reader.
 * @param context Where the array is allocated.
 * @return array The contents of what @p location names, in the data type of
 * its file.
 */
REXLIB_API
array read(
	const image_location &location,
	image_reader_provider &readers,
	const execution_context &context
);

/**
 * @brief Read one element per location into a batch, asynchronously.
 *
 * Turns a batch of locations into an image_transaction_plan and hands it to
 * @p source. Each slot is either the slice
 * @ref image_location::get_index_in_stack names within its file, or the
 * whole file when a location carries none. A batch may not mix the two;
 * every location must either carry an index in a stack or none may.
 *
 * Returns before the reads are done, unlike @ref read, and fills an array
 * the caller already has rather than allocating one.
 *
 * @param source Where the reads are dispatched. Needs to outlive this call
 * and no longer, the work outliving it carrying what it needs.
 * @param destination Where the elements land. Its leading extent is the
 * batch size and its remaining extents are the shape of one element.
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
 * slice @ref image_location::get_index_in_stack indexes within the file or
 * the whole file when it carries no index in a stack.
 *
 * @par Centres
 * A patch spans from @c centre-extent/2 along each axis, so its centre lands
 * at index @c extent/2 within it. That is the middle sample for an odd extent
 * and the origin a Fourier transform of the patch would use for an even one.
 *
 * @par Borders
 * A patch reaching past the edge of the image is read as far as the image
 * goes and no further, which @ref image_source resolves. The elements of
 * @p destination no data reached are left untouched, so what a patch is
 * padded with is decided by what @p destination held beforehand. Filling it
 * beforehand with a sentinel that cannot occur in the image, such as a quiet
 * NaN, is what lets a later pass tell the padding apart and replace it.
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
