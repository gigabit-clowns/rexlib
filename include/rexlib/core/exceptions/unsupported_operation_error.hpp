// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief Exception indicating that nothing available can carry out a
 * requested operation.
 *
 * Thrown when the implementations at hand, such as the registered backends
 * or formats, support none of what was asked for: an operation on those
 * operands, a file of that kind, a conversion between those data types.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API unsupported_operation_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace rexlib
