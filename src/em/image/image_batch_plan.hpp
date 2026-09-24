// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

class image_location;
class image_transaction_plan;

/**
 * @brief Make the transaction for a batch of whole elements.
 *
 * One region per slot of the array, in the order the locations are given.
 * Each is either the slice @ref image_location::get_index_in_stack names
 * within its file, or the whole file when a location carries no index in a
 * stack. A batch may not mix the two, since they do not agree on the rank of
 * the file.
 *
 * The leading extent of the array is the batch size and the rest are the
 * shape of one element.
 *
 * Shared so that @ref read_batch_async and @ref write_batch_async address a
 * batch identically, only the direction the transaction is handed to
 * differing.
 *
 * @param array_extents Extents of the array, its leading one the batch size.
 * @param locations Where each slot comes from, or goes to.
 * @param context What a message names as the caller, the calling method.
 * @return image_transaction_plan The transaction, one region per location.
 * @throws std::invalid_argument If @p array_extents is empty, if its leading
 * extent is not the number of locations, or if @p locations mixes those
 * carrying an index in a stack with those carrying none.
 */
image_transaction_plan make_batch_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations,
	const char *context
);

} // namespace em
} // namespace rexlib
