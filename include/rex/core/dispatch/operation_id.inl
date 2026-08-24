// SPDX-License-Identifier: GPL-3.0-only

#include "operation_id.hpp"

namespace rexlib
{

inline
operation_id::operation_id(std::type_index id) noexcept
	: m_id(id)
{
}

inline
std::size_t operation_id::hash() const noexcept
{
	return std::hash<std::type_index>()(m_id);
}

template<typename T>
inline
operation_id operation_id::of() noexcept
{
	using operation_type = typename std::decay<T>::type;
	return operation_id(typeid(operation_type));
}

} // namespace rexlib

namespace std
{

inline
std::size_t
hash<rexlib::operation_id>::operator()
(const rexlib::operation_id &key) const noexcept
{
	return key.hash();
}

} // namespace std
