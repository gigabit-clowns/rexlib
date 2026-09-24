// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_region_read_plan.hpp"
#include "image_region_write_plan.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

/**
 * @brief Move every region of a batch out of a file and into an array.
 *
 * Values are converted to @p array_type, and read in @p file_order.
 *
 * @param plan The regions and the space they are walked in.
 * @param array_data First element of the array.
 * @param array_type Data type of the array.
 * @param file_data First element of the values of the file.
 * @param file_type Data type of the file.
 * @param file_order Byte order the file states its values in.
 * @throws invalid_operation_error If @p array_type can not be produced from
 * @p file_type, or if @p file_type is not a data type files are
 * transferred in.
 */
void read_regions(
	const image_region_read_plan &plan,
	void *array_data,
	numerical_type array_type,
	const byte *file_data,
	numerical_type file_type,
	byte_order file_order
);

/**
 * @brief Move a run of the regions of a batch out of a file and into an array.
 *
 * The regions of a batch are held in ascending file order, so a run of them is
 * a stretch of the file, which is what lets a reader walk a batch a step at a
 * time and advise the step after the one it is walking.
 *
 * @param plan The regions and the space they are walked in.
 * @param first_region Index of the first region to move, among those @p plan
 * holds.
 * @param region_count How many regions to move from there. The run must not
 * reach past what @p plan holds.
 * @param array_data First element of the array.
 * @param array_type Data type of the array.
 * @param file_data First element of the values of the file.
 * @param file_type Data type of the file.
 * @param file_order Byte order the file states its values in.
 * @throws invalid_operation_error If @p array_type can not be produced from
 * @p file_type, or if @p file_type is not a data type files are
 * transferred in.
 */
void read_regions(
	const image_region_read_plan &plan,
	std::size_t first_region,
	std::size_t region_count,
	void *array_data,
	numerical_type array_type,
	const byte *file_data,
	numerical_type file_type,
	byte_order file_order
);

/**
 * @brief Move every region of a batch out of an array and into a file.
 *
 * The mirror of @ref read_regions, over a plan whose layout is ordered for
 * the file instead.
 *
 * @param plan The regions and the space they are walked in.
 * @param array_data First element of the array.
 * @param array_type Data type of the array.
 * @param file_data First element of the values of the file.
 * @param file_type Data type of the file.
 * @param file_order Byte order the file states its values in.
 * @throws invalid_operation_error If @p file_type can not be produced from
 * @p array_type, or if @p file_type is not a data type files are
 * transferred in.
 */
void write_regions(
	const image_region_write_plan &plan,
	const void *array_data,
	numerical_type array_type,
	byte *file_data,
	numerical_type file_type,
	byte_order file_order
);

/**
 * @brief Move a batch out of a file of one statically known element type.
 *
 * Defined in image_region_transfer_impl.hpp and explicitly instantiated once
 * per data type files are transferred in, in the
 * image_region_transfer_<type>.cpp files. The array side is dispatched over
 * every data type there is, and each pair of types drives a loop specialized
 * on the strides of both operands, so one element type is already a grid the
 * size of the CPU copy builder's. A translation unit per element type keeps
 * each of them well inside it.
 *
 * @tparam Q Element type of the file.
 * @param plan The regions and the space they are walked in.
 * @param array_data First element of the array.
 * @param array_type Data type of the array.
 * @param file_data First element of the values of the file.
 * @param swapped Whether the file states its values in the other byte order.
 * @throws invalid_operation_error If @p array_type can not be produced from
 * @p Q.
 */
template <typename Q>
void read_regions_as(
	const image_region_read_plan &plan,
	std::size_t first_region,
	std::size_t region_count,
	void *array_data,
	numerical_type array_type,
	const Q *file_data,
	bool swapped
);

/**
 * @brief Move a batch into a file of one statically known element type.
 *
 * @see read_regions_as
 */
template <typename Q>
void write_regions_as(
	const image_region_write_plan &plan,
	const void *array_data,
	numerical_type array_type,
	Q *file_data,
	bool swapped
);

} // namespace em
} // namespace rexlib
