// SPDX-License-Identifier: GPL-3.0-only

#include "operation_id.hpp"

namespace rex
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

} // namespace rex

namespace std
{

inline
std::size_t
hash<xmipp4::operation_id>::operator()
(const xmipp4::operation_id &key) const noexcept
{
	return key.hash();
}

} // namespace std
