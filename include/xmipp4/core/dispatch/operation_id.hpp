// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <typeindex>
#include <functional>
#include <cstddef>

namespace rex
{

/**
 * @brief Unique identification for an operation type.
 */
class operation_id
{
public:
	explicit operation_id(std::type_index id) noexcept;
	operation_id(const operation_id &other) = default;
	operation_id(operation_id &&other) = default;
	~operation_id() = default;

	operation_id& operator=(const operation_id &other) = default;
	operation_id& operator=(operation_id &&other) = default;
	
	friend bool
	operator==(const operation_id &lhs, const operation_id &rhs) noexcept
	{
		return lhs.m_id == rhs.m_id;
	}

	friend bool
	operator!=(const operation_id &lhs, const operation_id &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Compute the hash for this operation identifier.
	 * 
	 * @return std::size_t The hash.
	 */
	std::size_t hash() const noexcept;

	template<typename T>
	static operation_id of() noexcept;

private:
	std::type_index m_id;
};

} // namespace rex

namespace std
{

template<>
struct hash<xmipp4::operation_id>
{
	using key_type = xmipp4::operation_id;
	std::size_t operator()(const key_type &k) const noexcept;
};

} // namespace std

#include "operation_id.inl"
