// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rex/core/platform/dynamic_shared_object.h>

namespace rex 
{

/**
 * @brief Exception indicating that the function call is not available due to
 * a misconfiguration of the class.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API invalid_operation_error : public std::logic_error
{
	using logic_error::logic_error;
};

} // namespace rex
