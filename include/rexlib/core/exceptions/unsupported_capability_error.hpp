// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief Exception indicating that an object lacks a capability a call
 * requires.
 *
 * Thrown when a call is valid in general but not for the object it is made
 * on, such as reaching storage from the host when the storage lives where
 * the host can not see it.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API unsupported_capability_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace rexlib
