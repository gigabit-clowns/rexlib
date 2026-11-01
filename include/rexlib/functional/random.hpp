// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <cstdint>

namespace rexlib
{

class execution_context;

/**
 * @brief Create a key from a seed.
 *
 * A key is a uint64 element. The key of a seed holds the seed unchanged.
 *
 * @param seed The seed.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array A uint64 array without axes, in device memory, holding the
 * key.
 */
REXLIB_API
array make_key(
	std::uint64_t seed,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Draw a seed from the entropy source of the system.
 *
 * The seed joins two draws of `std::random_device`, so all of its 64 bits
 * are random. Each call gives a different seed, and the result can be
 * recorded to repeat a computation.
 *
 * @return std::uint64_t The seed.
 *
 * @throws std::exception When `std::random_device` cannot reach an entropy
 * source.
 */
REXLIB_API
std::uint64_t entropy_seed();

/**
 * @brief Derive keys from keys and values.
 *
 * @param key The uint64 keys.
 * @param data The uint64 values folded into the keys. It broadcasts against
 * @p key.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The derived keys.
 *
 * @see ops::fold_in_key_operation
 */
REXLIB_API
array fold_in_key(
	const_array_ref key,
	const_array_ref data,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Derive a sequence of keys from every key.
 *
 * @param key The uint64 keys.
 * @param count The number of keys derived from every key.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The derived keys: the shape of @p key followed by one axis
 * of @p count elements.
 *
 * @see ops::split_key_operation
 */
REXLIB_API
array split_key(
	const_array_ref key,
	std::size_t count,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
