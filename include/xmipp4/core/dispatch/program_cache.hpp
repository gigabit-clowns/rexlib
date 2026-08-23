// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_cache_key.hpp"

#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <typeindex>

namespace rex
{

/**
 * @brief Bounded associative store for backend-private resources reused
 * across operation program builds.
 *
 * The cache behaves as a typed key/value bag: each builder defines its own
 * key type and stores its own backend-specific resources (FFT plans,
 * compiled kernels, ...). Entries are partitioned by the static type of
 * the key, so unrelated builders never collide even if their key layouts
 * happen to coincide.
 *
 * The cache holds at most a fixed number of entries set at construction;
 * once that bound is reached, inserting a new entry evicts the oldest one.
 * Eviction order is shared across all key types: a hot inner bucket may
 * evict entries from a cold one.
 *
 * @par Thread safety
 * Individual calls to touch() and store() are atomic and may be issued
 * concurrently from multiple threads. The compound touch-then-store sequence
 * a builder performs on a miss is intentionally *not* atomic as a whole:
 * because the cache stores functionally interchangeable resources, two
 * threads racing on the same missing key may each build the resource and
 * store it, with one replacing the other. This wastes a build but never
 * corrupts the cache, and either stored value is equally valid.
 */
class program_cache
{
public:
	/**
	 * @brief Construct a cache with the given capacity.
	 *
	 * @param capacity Maximum number of entries the cache may hold. Must
	 * be greater than zero.
	 */
	REXLIB_API
	explicit program_cache(std::size_t capacity) noexcept;
	program_cache(const program_cache &other) = delete;
	program_cache(program_cache &&other) = delete;
	REXLIB_API
	~program_cache();

	program_cache&
	operator=(const program_cache &other) = delete;
	program_cache&
	operator=(program_cache &&other) = delete;

	/**
	 * @brief Get the maximum number of entries this cache may hold.
	 *
	 * @return std::size_t The capacity, in entries.
	 */
	REXLIB_API
	std::size_t get_capacity() const noexcept;

	/**
	 * @brief Look up a previously stored value and grant it amnesty.
	 *
	 * Lookup is partitioned by the dynamic type of @p key, so entries
	 * stored under keys of a different dynamic type are never considered.
	 * On a hit, the entry is promoted to the lowest eviction priority, so
	 * recently touched entries are the last to be evicted.
	 *
	 * @param key The key to look up. Its dynamic type and value must match
	 * those of the key under which the entry was stored.
	 * @return std::shared_ptr<void> The stored value, or nullptr if no
	 * entry is present.
	 */
	REXLIB_API
	std::shared_ptr<void> touch(const program_cache_key &key);

	/**
	 * @brief Store a value under the given key, replacing any existing
	 * entry under a key of the same dynamic type and value. May evict
	 * another entry when the cache is at capacity.
	 *
	 * The newly stored or replaced entry is placed at the lowest eviction
	 * priority.
	 *
	 * @param key The key under which the value is stored. The cache takes
	 * ownership. Must not be null.
	 * @param value The value to store. Must not be null.
	 * @throws std::invalid_argument When @p key or @p value is null.
	 */
	REXLIB_API
	void store(
		std::unique_ptr<const program_cache_key> key,
		std::shared_ptr<void> value
	);

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;
};

} // namespace rex
