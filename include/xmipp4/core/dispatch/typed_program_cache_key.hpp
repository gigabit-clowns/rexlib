// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_cache_key.hpp"

#include <memory>
#include <type_traits>

namespace rex
{

/**
 * @brief Default key wrapper for value types that model std::hash and
 * equality comparison.
 *
 * @tparam K The wrapped key type. Must be hashable through std::hash and
 * equality-comparable
 */
template <typename K>
class typed_program_cache_key final
	: public program_cache_key
{
public:
	explicit typed_program_cache_key(K key);
	~typed_program_cache_key() override = default;

	std::size_t hash() const noexcept override;
	bool equals(
		const program_cache_key &other
	) const noexcept override;

	const K& get_key() const noexcept;

private:
	K m_key;
};

} // namespace rex

#include "typed_program_cache_key.inl"
