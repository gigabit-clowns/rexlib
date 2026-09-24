// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <cstddef>
#include <memory>

namespace rexlib
{

class completion;

namespace em
{

class image_descriptor;
class image_location;
class image_sink;

/**
 * @brief Write a whole array out as one image or volume.
 *
 * Creates the file, writes every element and flushes it, all before
 * returning. The file holds a single image or volume with the extents of
 * @p arr. Its asynchronous peer below writes into files declared elsewhere.
 *
 * @param arr The values to write.
 * @param path Path to the file to create.
 * @param manager The formats the file may be created with.
 * @param data_type Data type of the file, the values of @p arr being
 * converted to it, or unknown to keep the one @p arr carries.
 * @param metadata How its samples map onto physical space.
 * @throws std::invalid_argument If @p arr has no extents, or if neither
 * @p data_type nor @p arr names a data type.
 *
 * @see write_stack
 */
REXLIB_API
void write_single(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	numerical_type data_type = numerical_type::unknown,
	const image_metadata &metadata = image_metadata()
);

/**
 * @brief Write a whole array out as a stack of images or volumes.
 *
 * As @ref write_single, except that the leading extent of @p arr is the axis
 * the file stacks along and the rest are the extents of one image or volume:
 * an array of @c (N,H,W) becomes a stack of @c N images of @c (H,W).
 *
 * @param arr The values to write.
 * @param path Path to the file to create.
 * @param manager The formats the file may be created with.
 * @param data_type Data type of the file, the values of @p arr being
 * converted to it, or unknown to keep the one @p arr carries.
 * @param metadata How its samples map onto physical space.
 * @throws std::invalid_argument If @p arr has fewer than two extents, or if
 * neither @p data_type nor @p arr names a data type.
 */
REXLIB_API
void write_stack(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	numerical_type data_type = numerical_type::unknown,
	const image_metadata &metadata = image_metadata()
);

/**
 * @brief Write a whole array out as the file a descriptor states.
 *
 * As @ref write_single, except that the file is created as @p descriptor
 * states. Its core rank is what makes the file a stack of images or volumes
 * rather than a single one, and its data type is what the file holds, the
 * values of @p arr being converted to it.
 *
 * @param arr The values to write. Its extents must be those of
 * @p descriptor.
 * @param path Path to the file to create.
 * @param manager The formats the file may be created with.
 * @param descriptor What the file holds.
 * @param metadata How its samples map onto physical space.
 * @throws std::invalid_argument If the extents of @p arr are not those of
 * @p descriptor.
 */
REXLIB_API
void write(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	const image_descriptor &descriptor,
	const image_metadata &metadata = image_metadata()
);

/**
 * @brief Write one element per location out of a batch, asynchronously.
 *
 * The mirror of @ref read_batch_async: it turns a batch of locations into an
 * image_transaction_plan the same way and hands it to @p sink. Each slot
 * becomes either the slice @ref image_location::get_index_in_stack names
 * within its file, or the whole file when a location carries none. A batch
 * may not mix the two.
 *
 * Returns before the writes are done, unlike @ref write, and writes into
 * files that already exist rather than creating one.
 *
 * @par Writing a stack a batch at a time
 * A file is created with its whole shape before anything is written, so the
 * size of a stack is stated once, when it is declared through
 * @ref managed_image_writer_provider::declare, and this writes into it a
 * batch at a time. Nothing binds a call to one stack: several may be written
 * at once and one batch may span more than one. Closing a stack is the
 * declaring side's business too, once the completion of every batch written
 * into it has resolved.
 *
 * Every slot must fit where it is written, a location naming an index the
 * stack does not hold being reported through the completion rather than
 * dropped. See @ref image_sink::write.
 *
 * @param sink Where the writes are dispatched. Needs to outlive this call
 * and no longer, the work outliving it carrying what it needs.
 * @param source The values to write. Its leading extent is the batch size
 * and its remaining extents are the shape of one element.
 * @param locations Where each slot goes, one per slot of @p source and in
 * the same order.
 * @return std::shared_ptr<completion> The completion, never null.
 * @throws std::invalid_argument If @p source has no extents, if its leading
 * extent is not the number of locations, or if @p locations mixes those
 * carrying an index in a stack with those carrying none.
 */
REXLIB_API
std::shared_ptr<completion> write_batch_async(
	const image_sink &sink,
	const_array source,
	span<const image_location> locations
);

} // namespace em
} // namespace rexlib
