// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class completion;
class const_array;

namespace em
{

class image_sink;
class image_location;

/**
 * @brief Writes one batch into as many image_location-s, asynchronously.
 *
 * The mirror of @ref image_batch_source: it turns a batch of locations into
 * an image_transaction_plan the same way and hands it to a downstream
 * @ref image_sink. Each slot becomes either the slice
 * @ref image_location::get_position_in_stack names within the file, or the
 * whole file when a location carries none. A batch may not mix the two;
 * every location must either carry a position or none may.
 *
 * The source array's leading extent is the batch size, one slot per location
 * in the order given, and its remaining extents are the shape of one
 * location's region, which must match at the file.
 *
 * @par Writing a stack in batches
 * A file is created with its whole shape before anything is written, so the
 * size of a stack is stated once, when it is declared through
 * @ref managed_image_writer_provider::declare, and this writes into it a
 * batch at a time. Nothing binds this to one stack: every call names its
 * own, so several may be written at once and one batch may span more than
 * one. Closing a stack is the declaring side's business too, once the
 * completion of every batch written into it has resolved.
 *
 * @par Thread safety
 * write may be called concurrently.
 */
class image_batch_sink
{
public:
	/**
	 * @brief Construct a batch sink over a downstream image sink.
	 *
	 * @param sink The downstream image_sink used for writing.
	 * @throws std::invalid_argument If @p sink is null.
	 */
	REXLIB_API
	explicit image_batch_sink(std::shared_ptr<const image_sink> sink);

	image_batch_sink(const image_batch_sink &other) = delete;
	image_batch_sink(image_batch_sink &&other) = delete;

	REXLIB_API
	~image_batch_sink();

	image_batch_sink& operator=(const image_batch_sink &other) = delete;
	image_batch_sink& operator=(image_batch_sink &&other) = delete;

	/**
	 * @brief Write one element per location out of a batch.
	 *
	 * Returns before the writes are done. Neither @ref completion::wait nor
	 * @ref completion::get of the completion returned may be called from
	 * within a task already running on the executor the downstream sink was
	 * constructed with.
	 *
	 * Every slot must fit where it is written, a location naming a position
	 * the stack does not hold being reported through the completion rather
	 * than dropped. See @ref image_sink::write.
	 *
	 * @param source The values to write. Its leading extent is the batch
	 * size and its remaining extents are the shape of one element.
	 * @param locations Where each slot goes, one per slot of @p source and
	 * in the same order.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p source has no extents, if its
	 * leading extent is not the number of locations, or if @p locations
	 * mixes those carrying a position with those carrying none.
	 */
	REXLIB_API
	std::shared_ptr<completion> write(
		const_array source,
		span<const image_location> locations
	) const;

private:
	std::shared_ptr<const image_sink> m_sink;
};

} // namespace em
} // namespace rexlib
