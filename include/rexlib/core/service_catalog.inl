// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "service_catalog.hpp"

#include "platform/constexpr.hpp"
#include "platform/assert.hpp"

namespace rexlib
{

template <typename T>
inline
std::shared_ptr<T> service_catalog::get_service()
{
	const std::type_index key(typeid(T));

	auto value = std::static_pointer_cast<T>(get_service(key));
	if(!value)
	{
		// The service does not exist yet: create it.
		value = std::make_shared<T>();
		create_service(key, value);
	}

	return value;
}

} // namespace rexlib
