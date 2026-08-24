// SPDX-License-Identifier: GPL-3.0-only

#include "array_implementation.hpp"

namespace rexlib 
{

inline
array_implementation::array_implementation(
	std::shared_ptr<buffer> storage,
	array_descriptor descriptor
) noexcept
	: m_storage(std::move(storage))
	, m_descriptor(std::move(descriptor))
{
}

inline
const array_descriptor& array_implementation::get_descriptor() const noexcept
{
	return m_descriptor;
}

inline
buffer* array_implementation::get_storage() const noexcept
{
	return m_storage.get();
}

inline
const std::shared_ptr<buffer>& 
array_implementation::share_storage() const noexcept
{
	return m_storage;
}

} // namespace rexlib
