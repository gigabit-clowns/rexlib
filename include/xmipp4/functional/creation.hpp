// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>

#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/numerical/scalar_value.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

#include <cstddef>

namespace xmipp4
{

class execution_context;


/**
 * @brief Create an array without initializing its elements.
 *
 * @param descriptor Describes the shape and data type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array empty(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create an array with all its elements set to zero.
 *
 * @param descriptor Describes the shape and data type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array zeros(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create an array with all its elements set to one.
 *
 * @param descriptor Describes the shape and data type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array ones(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create an array with all its elements set to a given value.
 *
 * @param descriptor Describes the shape and data type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param fill_value Value to which all the elements are set.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array full(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const scalar_value &fill_value,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create a one dimensional array holding an arithmetic progression.
 *
 * The elements are `start`, `start + step`, `start + 2 * step` and so on,
 * stopping before @p stop is reached or passed, so that the range is half
 * open exactly as a loop over it would be. The length follows from the three
 * of them and is `ceil((stop - start) / step)`, or zero when the step points
 * away from the stop.
 *
 * The length is worked out in whole numbers when all three bounds are
 * integers, and in double precision otherwise. This mirrors what a loop over
 * the range would count, and is what keeps a purely integral range exact
 * regardless of how large its bounds are.
 *
 * @param start Value of the first element.
 * @param stop Value the progression stops before.
 * @param step Difference between one element and the next.
 * @param data_type Element type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 *
 * @throws std::invalid_argument If the step is zero, or if the length the
 * bounds describe is not a finite number.
 */
REXLIB_API
array arange(
	const scalar_value &start,
	const scalar_value &stop,
	const scalar_value &step,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create a one dimensional array counting from start to stop by one.
 *
 * @param start Value of the first element.
 * @param stop Value the progression stops before.
 * @param data_type Element type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array arange(
	const scalar_value &start,
	const scalar_value &stop,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create a one dimensional array counting from zero to stop by one.
 *
 * @param stop Value the progression stops before.
 * @param data_type Element type of the array.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array arange(
	const scalar_value &stop,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create a one dimensional array of evenly spaced samples.
 *
 * Takes @p count samples of the segment going from @p start to @p stop, both
 * ends included. Unlike @ref arange the length is what the caller states and
 * the spacing is what follows from it, which is why the last element is
 * exactly @p stop however the spacing rounds.
 *
 * @param start Value of the first element.
 * @param stop Value of the last element.
 * @param count Number of samples.
 * @param data_type Element type of the array. It must be a floating point or
 * complex type: evenly spaced samples generally have a fractional part, and
 * @ref arange is the one that enumerates whole numbers.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array linspace(
	const scalar_value &start,
	const scalar_value &stop,
	std::size_t count,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create a one dimensional array of evenly spaced samples.
 *
 * @param start Value of the first element.
 * @param stop End of the sampled segment.
 * @param count Number of samples.
 * @param endpoint Whether @p stop is itself a sample. When it is not, the
 * samples are those of the half open segment, so that consecutive blocks of
 * them tile the line without repeating a value.
 * @param data_type Element type of the array. It must be a floating point or
 * complex type.
 * @param affinity Affinity used to select the memory resource where the
 * array is allocated.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array linspace(
	const scalar_value &start,
	const scalar_value &stop,
	std::size_t count,
	bool endpoint,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Create an array as a copy of an existing one.
 *
 * @param source The array to be copied.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The newly created array.
 */
REXLIB_API
array copy(
	const_array_ref source,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Set all the elements of an array to a given value.
 *
 * @param out The array whose elements are set.
 * @param fill_value Value to which all the elements are set.
 * @param context The execution context used for dispatching.
 * @return array The filled array.
 */
REXLIB_API
void fill(
	array &out,
	const scalar_value &fill_value,
	const execution_context &context
);

} // namespace xmipp4
