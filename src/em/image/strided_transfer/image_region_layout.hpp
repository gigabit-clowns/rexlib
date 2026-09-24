// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

class image_transfer_plan;

/**
 * @brief Build the space every region of a batch is walked in.
 *
 * @ref joint_layout orders its axes by the strides of its operands taken in
 * the order they were added, the first one to prefer an order deciding it and
 * the rest only breaking ties. The destination is therefore named first, so
 * that the traversal is made sequential in the side being written: a
 * scattered write through a mapping dirties its pages out of order, and what
 * that costs on the way back to the storage has no equivalent on the read
 * side.
 *
 * Both sides are given as the strides of the whole side. The extents of a
 * batch cover only its trailing axes, so only that many strides are taken.
 *
 * @param regions The batch whose extents the space has.
 * @param destination_strides Strides of the side being written, in elements.
 * @param source_strides Strides of the side being read, in elements.
 * @return joint_layout The space, naming the destination first.
 */
joint_layout build_region_layout(
	const image_transfer_plan &regions,
	span<const std::ptrdiff_t> destination_strides,
	span<const std::ptrdiff_t> source_strides
);

} // namespace em
} // namespace rexlib
