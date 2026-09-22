// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>
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

/**
 * @brief Loads a batch of image_location-s into one array, asynchronously.
 *
 * Turns a batch of locations into an image_transaction_plan and hands it to
 * a downstream image_source. Its region is either the slice indexed by
 * @ref image_location::get_position_in_stack names within the file, or the
 * whole file when a location carries none. A batch may not mix the two;
 * every location must either carry a position or none may.
 *
 * The destination array's leading extent is the batch size, one slot per
 * location in the order given, and its remaining extents are the shape of
 * one location's region, which must match at the image.
 */
class image_batch_source
{
public:
	/**
	 * @brief Construct a batch source over a provider image source.
	 *
	 * @param source The downstream image_source used for reading.
	 * @throws std::invalid_argument If @p source is null.
	 */
	REXLIB_API
	explicit image_batch_source(std::shared_ptr<const image_source> source);

	image_batch_source(const image_batch_source &other) = delete;
	image_batch_source(image_batch_source &&other) = delete;

	REXLIB_API
	~image_batch_source();

	image_batch_source& operator=(const image_batch_source &other) = delete;
	image_batch_source& operator=(image_batch_source &&other) = delete;

	/**
	 * @brief Read one element per location into a batch.
	 *
	 * Returns before the reads are done. Neither @ref completion::wait
	 * nor @ref completion::get of the completion returned may be called
	 * from within a task already running on the executor this source
	 * was constructed with.
	 *
	 * @param destination Where the elements land. Its leading extent is the
	 * batch size and its remaining extents are the shape of one element.
	 * @param locations Where each slot comes from, one per slot of
	 * @p destination and in the same order.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p destination has no extents, if its
	 * leading extent is not the number of locations, or if @p locations
	 * mixes those carrying a position with those carrying none.
	 */
	REXLIB_API
	std::shared_ptr<completion> read(
		array destination,
		span<const image_location> locations
	) const;

private:
	std::shared_ptr<const image_source> m_source;
};

} // namespace em
} // namespace rexlib
