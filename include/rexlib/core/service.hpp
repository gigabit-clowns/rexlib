// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/dynamic_shared_object.h"


namespace rexlib
{

/**
 * @brief Base class of the services a @ref service_catalog holds.
 *
 * A service keeps track of the implementations of one kind that plugins
 * register, such as device backends or program builders, and gives access
 * to the one that suits a request.
 *
 * @see service_catalog
 */
class REXLIB_API service
{
public:
	service() = default;
	service(const service& other) = delete;
	service(service&& other) = delete;
	virtual ~service() = default;

	service& operator=(const service& other) = delete;
	service& operator=(service&& other) = delete;
};

} // namespace rexlib
