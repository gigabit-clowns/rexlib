// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/dynamic_shared_object.h"


namespace xmipp4
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

	/**
	 * @brief Register backends bundled with the core library.
	 * 
	 * This usually loads fallback implementations that are always 
	 * available although they may not be the most efficient or performant.
	 * The loaded backend(s) depend on the implementation of the specific
	 * interface.
	 * 
	 */
	virtual void register_builtin_backends() { /*By default this is a no-op*/ };
};

} // namespace xmipp4
