// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <rexlib/core/layout/dynamic_subscript.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Obtain a view of a rectangular part of an array.
 *
 * The result shares the storage of @p input and describes it through the
 * layout that @p subscripts selects, so writing through the result writes
 * through @p input. Nothing is allocated and no element is touched: this is
 * a reinterpretation of the extents, the strides and the offset, which is
 * why it needs neither an execution context nor an output parameter.
 *
 * The subscripts are the same ones @ref strided_layout::apply_subscripts
 * accepts, so an axis may be indexed away, sliced with a step, kept whole
 * with @ref ellipsis_tag or introduced with @ref new_axis_tag. Axes left
 * unaddressed are kept whole, so fewer subscripts than the array has axes
 * views the remaining ones in full.
 *
 * @param input The array to view into. Its storage outlives the result,
 * which shares ownership of it.
 * @param subscripts The subscripts selecting the part of interest.
 * @return array A view of the selected part of @p input.
 * @throws std::invalid_argument If @p input is not initialized or if there
 * are more subscripts than the array has axes.
 * @throws std::out_of_range If a subscript falls outside its axis.
 *
 * @see strided_layout::apply_subscripts
 */
REXLIB_API
array subarray(array &input, span<const dynamic_subscript> subscripts);

/**
 * @brief Obtain a read-only view of a rectangular part of an array.
 *
 * Behaves as the non-const overload, except that the result only allows
 * reading. It accepts anything a @ref const_array_ref accepts, so it is also
 * the overload chosen for a const array.
 *
 * @param input The array to view into. Its storage outlives the result,
 * which shares ownership of it.
 * @param subscripts The subscripts selecting the part of interest.
 * @return const_array A read-only view of the selected part of @p input.
 * @throws std::invalid_argument If @p input is not initialized or if there
 * are more subscripts than the array has axes.
 * @throws std::out_of_range If a subscript falls outside its axis.
 *
 * @see strided_layout::apply_subscripts
 */
REXLIB_API
const_array subarray(
	const_array_ref input,
	span<const dynamic_subscript> subscripts
);

} // namespace rexlib
