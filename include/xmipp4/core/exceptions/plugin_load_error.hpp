// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdexcept>

#include <xmipp4/core/platform/dynamic_shared_object.h>

namespace xmipp4 
{

/**
 * @brief Exception indicating that an error occurred when loading a plugin.
 */
REX_STD_BASE_INTERFACE
class REXLIB_API plugin_load_error : public std::runtime_error
{
	using runtime_error::runtime_error;
};

} // namespace xmipp4
