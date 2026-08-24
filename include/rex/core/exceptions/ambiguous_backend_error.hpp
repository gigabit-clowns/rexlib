// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <rex/core/platform/dynamic_shared_object.h>

namespace rex 
{

/**
 * @brief Exception indicating that could not disambiguate among multiple
 * backend candidates.
 */
REXLIB_STD_BASE_INTERFACE
class REXLIB_API ambiguous_backend_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace rex
