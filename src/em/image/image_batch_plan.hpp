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
 * Each is either the slice @ref image_location::get_position_in_stack names
 * within its file, or the whole file when a location carries no position. A
 * batch may not mix the two, since they do not agree on the rank of the file.
 *
 * The leading extent of the array is the batch size and the rest are the
 * shape of one element.
 *
 * A batch naming consecutive positions of one file from end to end, which is
 * what a stack read or written a batch at a time is, travels as a single
 * region rather than as one per slot. A plan carries one set of extents for
 * every region it holds, so a batch only partly made of such neighbours
 * cannot merge the part that is and stays one region per slot, as does a
 * batch of whole files, no two of which are ever consecutive.
 *
 * Shared so that @ref image_batch_source and @ref image_batch_sink address a
 * batch identically, only the direction the transaction is handed to
 * differing.
 *
 * @param array_extents Extents of the array, its leading one the batch size.
 * @param locations Where each slot comes from, or goes to.
 * @param context What a message names as the caller, the calling method.
 * @return image_transaction_plan The transaction, one region per location.
 * @throws std::invalid_argument If @p array_extents is empty, if its leading
 * extent is not the number of locations, or if @p locations mixes those
 * carrying a position with those carrying none.
 */
image_transaction_plan make_batch_transaction_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations,
	const char *context
);

} // namespace em
} // namespace rexlib
