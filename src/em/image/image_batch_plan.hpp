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
 * @brief Make the plan that pairs each slot along the leading axis of an
 * array with the image or volume a location names.
 *
 * One region per slot, in the order the locations are given. Each is either
 * the image or volume @ref image_location::get_index_in_stack indexes within
 * its file, or the whole file when a location carries no index in a stack.
 * The locations may not mix the two, since they do not agree on the rank of
 * the file.
 *
 * The leading extent of the array is the number of slots and the rest are
 * the shape of one image or volume. The plan describes a read and a write
 * alike.
 *
 * @param array_extents Extents of the array, its leading one the number of
 * slots.
 * @param locations Where each slot comes from, or goes to.
 * @param context What an error message starts with.
 * @return image_transaction_plan The plan, one region per location.
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
