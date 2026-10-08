// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

class array_ref;
class const_array_ref;

namespace em
{

class image_transfer_plan;

/**
 * @brief Copy regions of one host array into another.
 *
 * @p source is the file side of the plan and @p destination its array side.
 * Each region is copied as @ref image_reader::read would read it out of a
 * file holding the values of @p source: either array may be strided, and
 * the values are converted to the data type of @p destination.
 *
 * An empty plan copies nothing.
 *
 * @param source The array to copy from. Must be initialized and host
 * accessible.
 * @param destination The array to copy into. Must be initialized and host
 * accessible.
 * @param regions The regions to copy and where each one lands.
 * @throws std::invalid_argument If either array is not initialized, or if
 * the offsets of the plan do not have the ranks of the arrays.
 * @throws std::out_of_range If a region is not contained in @p source, or
 * does not fit in @p destination where it is placed.
 * @throws unsupported_operation_error If the data type of @p destination
 * can not be produced from the one of @p source.
 * @throws unsupported_capability_error If either array is not host
 * accessible.
 */
void copy_regions(
	const_array_ref source,
	array_ref destination,
	const image_transfer_plan &regions
);

} // namespace em
} // namespace rexlib
