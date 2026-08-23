// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array_ref.hpp>

#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace rex
{

class execution_context;


/**
 * @brief Transfer an array to memory with the requested affinity.
 *
 * If the array is already accessible from a memory resource with the
 * requested affinity, this operation returns an alias of the input array.
 * Otherwise it behaves like @ref transfer_copy.
 *
 * @param input Array to be transferred.
 * @param affinity Affinity of the memory resource to transfer the array to.
 * @param context The execution environment.
 * @return array The transferred array.
 *
 * @warning The returned array may alias the input array. Thus modifications
 * to it may have side-effects on the input array and vice-versa. Prefer
 * @ref transfer_copy if this is a concern.
 */
REXLIB_API
array transfer(
	array &input,
	memory_resource_affinity affinity,
	const execution_context &context
);

/**
 * @brief Transfer an array to memory with the requested affinity enforcing a
 * copy.
 *
 * Unlike @ref transfer, this operation always allocates fresh storage and
 * copies the input into it, so the result never aliases the input.
 *
 * @param input Array to be transferred.
 * @param affinity Affinity of the memory resource to transfer the array to.
 * @param context The execution environment.
 * @param out Optional array to reuse. If provided, its resources may be re-used
 * and it will be overwritten with the newly created array.
 * @return array The transferred array.
 */
REXLIB_API
array transfer_copy(
	const_array_ref input,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transfer an array from host to device memory.
 * 
 * If the array is already device-accessible, this operation returns an
 * alias of the input array. Otherwise it behaves like `to_device_copy`
 * 
 * @param input Array to be transferred.
 * @param context The execution environment.
 * @return array The transferred array.
 * 
 * @warning The returned array may alias the input array. Thus modifications
 * to it may have side-effects on the input array and vice-versa. Prefer 
 * @ref to_device_copy if this is a concern.
 */
REXLIB_API
array to_device(
	array &input,
	const execution_context &context
);

/**
 * @brief Transfer an array to device memory enforcing a copy.
 * 
 * @param input Array to be transferred.
 * @param context The execution environment.
 * @param out Optional array to reuse. If provided, its resources may be re-used
 * and it will be overwritten with the newly created array.
 * @return array The transferred array.
 */
REXLIB_API
array to_device_copy(
	const_array_ref input,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Transfer an array to host accessible memory.
 * 
 * If the array is already host-accessible, this operation returns an
 * alias of the input array. Otherwise it behaves like `to_host_copy`
 * 
 * @param input Array to be transferred.
 * @param context The execution environment.
 * @return array The transferred array.
 * 
 * @warning The returned array may alias the input array. Thus modifications
 * to it may have side-effects on the input array and vice-versa. Prefer 
 * @ref to_host_copy if this is a concern. 
 */
REXLIB_API
array to_host(
	array &input,
	const execution_context &context
);

/**
 * @brief Transfer an array to host accessible memory enforcing a copy.
 * 
 * @param input Array to be transferred.
 * @param context The execution environment.
 * @param out Optional array to reuse. If provided, its resources may be re-used
 * and it will be overwritten with the newly created array.
 * @return array The transferred array.
 */
REXLIB_API
array to_host_copy(
	const_array_ref input,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rex
