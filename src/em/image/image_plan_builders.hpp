// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>

namespace rexlib
{

class index_table;

namespace em
{

class image_descriptor;
class image_location;

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
 * @return image_transaction_plan The plan, one region per location.
 * @throws std::invalid_argument If @p array_extents is empty, if its leading
 * extent is not the number of locations, or if @p locations mixes those
 * carrying an index in a stack with those carrying none.
 */
image_transaction_plan make_batch_plan(
	span<const std::size_t> array_extents,
	span<const image_location> locations
);

/**
 * @brief Make the plan that pairs each slot along the leading axis of an
 * array with a patch of one image or volume.
 *
 * One region per centre, in the order the centres are given, all of the
 * image or volume @p location names: the one
 * @ref image_location::get_index_in_stack indexes within its file, or the
 * whole file when it carries no index in a stack.
 *
 * The leading extent of the array is the number of slots and the rest are
 * the shape of one patch. A patch spans from its centre less half its extent
 * along each axis. Where that falls before the image begins, the region
 * starts at the edge of the image and further into its slot instead, so
 * every region keeps the extents of a whole patch. A patch may still reach
 * past the far edge of the image or of its slot.
 *
 * The plan describes a read and a write alike.
 *
 * @param array_extents Extents of the array, its leading one the number of
 * slots.
 * @param location The image or volume every patch belongs to.
 * @param centres Centre of each patch, of the rank of one patch.
 * @return image_transaction_plan The plan, one region per centre.
 * @throws std::invalid_argument If @p array_extents is empty, if its leading
 * extent is not the number of centres, or if the centres do not have the
 * rank of one patch.
 */
image_transaction_plan make_patch_plan(
	span<const std::size_t> array_extents,
	const image_location &location,
	const index_table &centres
);

/**
 * @brief Make the plan that pairs what a location names in a file with an
 * array of exactly its shape.
 *
 * One region, at the origin of the array. It spans the whole file when the
 * location carries no index in a stack, and the array then has the rank of
 * the file. Otherwise it spans the image or volume
 * @ref image_location::get_index_in_stack indexes along the leading axis of
 * the file, and the array has the rank of one image or volume.
 *
 * The plan describes a read and a write alike.
 *
 * @param file What the file holds.
 * @param location What of the file the region spans.
 * @return image_transfer_plan The plan, of one region.
 */
image_transfer_plan make_location_plan(
	const image_descriptor &file,
	const image_location &location
);

} // namespace em
} // namespace rexlib
