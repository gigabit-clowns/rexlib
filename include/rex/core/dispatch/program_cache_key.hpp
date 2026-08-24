// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Abstract base for keys stored in an @ref program_cache.
 */
class REXLIB_API program_cache_key
{
public:
	program_cache_key() noexcept;
	program_cache_key(
		const program_cache_key &other
	) = delete;
	program_cache_key(
		program_cache_key &&other
	) = delete;
	virtual ~program_cache_key();

	program_cache_key&
	operator=(const program_cache_key &other) = delete;
	program_cache_key&
	operator=(program_cache_key &&other) = delete;

	/**
	 * @brief Hash of the wrapped key.
	 *
	 * @return std::size_t The hash value.
	 */
	virtual std::size_t hash() const noexcept = 0;

	/**
	 * @brief Equality against another key.
	 *
	 * The cache guarantees that this method is only invoked with keys of the
	 * same type. Implementations may rely on that precondition and skip any
	 * runtime type check.
	 *
	 * @param other The key to compare against.
	 * @return true Both keys refer to the same entry.
	 * @return false The keys differ in value.
	 */
	virtual
	bool equals(const program_cache_key &other) const noexcept = 0;
};

} // namespace rexlib
