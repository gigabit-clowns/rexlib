// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class array;
class completion;

namespace em
{

class image_source;
class image_location;
class index_table;

/**
 * @brief Crops a batch of equally sized patches out of one image,
 * asynchronously.
 *
 * Turns a batch of positions within one image into an
 * @ref image_transaction_plan and hands it to a downstream
 * @ref image_source. Every patch comes from the image the one
 * @ref image_location names, which is either the slice
 * @ref image_location::get_position_in_stack indexes within the file or the
 * whole file when the location carries no position.
 *
 * The destination array's leading extent is the batch size, one slot per
 * position in the order given, and its remaining extents are the shape of one
 * patch, which is the same for every patch of the batch.
 *
 * @par Positions
 * A position is the centre of the patch it addresses, not its corner: the
 * patch spans from @c position-extent/2 along each axis, so the position
 * lands at index @c extent/2 within the patch. That is the middle sample for
 * an odd extent and the origin a Fourier transform of the patch would use for
 * an even one.
 *
 * @par Borders
 * A patch reaching past the edge of the image is read as far as the image
 * goes and no further, which @ref image_source resolves. The elements of the
 * destination no data reached are left untouched, so what a patch is padded
 * with is decided by what the destination held beforehand. Previously filling 
 * it with a sentinel value that should not occur in the image, such as a quiet 
 * NaN, is  what lets a later pass tell the padding apart and replace it.
 *
 * @par Thread safety
 * `read` may be called concurrently.
 */
class image_patch_source
{
public:
	/**
	 * @brief Construct a patch source over a provider image source.
	 *
	 * @param source The downstream image_source used for reading.
	 * @throws std::invalid_argument If @p source is null.
	 */
	REXLIB_API
	explicit image_patch_source(std::shared_ptr<const image_source> source);

	image_patch_source(const image_patch_source &other) = delete;
	image_patch_source(image_patch_source &&other) = delete;

	REXLIB_API
	~image_patch_source();

	image_patch_source& operator=(const image_patch_source &other) = delete;
	image_patch_source& operator=(image_patch_source &&other) = delete;

	/**
	 * @brief Read one patch per position of a batch.
	 *
	 * Returns before the reads are done. Neither @ref completion::wait nor
	 * @ref completion::get of the completion returned may be called from
	 * within a task already running on the executor the downstream source
	 * was constructed with.
	 *
	 * @param destination Where the patches land. Its leading extent is the
	 * batch size and its remaining extents are the shape of one patch.
	 * @param location The image every patch is cropped from.
	 * @param positions Centre of each patch, of the rank of one patch and as
	 * many as the batch size.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p destination has no extents, if
	 * @p positions does not hold one position per slot of @p destination, or
	 * if a position does not have the rank of one patch.
	 */
	REXLIB_API
	std::shared_ptr<completion> read(
		array destination,
		const image_location &location,
		const index_table &positions
	) const;

private:
	std::shared_ptr<const image_source> m_source;
};

} // namespace em
} // namespace rexlib
