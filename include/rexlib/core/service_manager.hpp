// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/dynamic_shared_object.h"


namespace rexlib
{

/**
 * @brief Abstract class defining the interface of a service manager.
 *
 * A service manager keeps track of all known implementations of a given
 * interface and provides utilities to access the most relevant implementation
 * for a given context. This is a base class for collecting them on an
 * service_catalog.
 *
 * @see service_catalog
 *
 */
class REXLIB_API service_manager
{
public:
	service_manager() = default;
	service_manager(const service_manager& other) = delete;
	service_manager(service_manager&& other) = delete;
	virtual ~service_manager() = default;

	service_manager& operator=(const service_manager& other) = delete;
	service_manager& operator=(service_manager&& other) = delete;
};

} // namespace rexlib
