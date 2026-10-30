// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace em
{

/**
 * @brief Component tag for the CryoEM operations.
 *
 * Every operation declaration names the component it belongs to, which
 * qualifies it in diagnostics and keeps the names of different components
 * apart.
 *
 * @see REXLIB_DECLARE_OPERATION
 */
struct em_component
{
	static const char* get_component() noexcept
	{
		return "rexlib.em";
	}
};

} // namespace em
} // namespace rexlib
