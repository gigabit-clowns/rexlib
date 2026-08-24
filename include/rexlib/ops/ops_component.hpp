// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace ops
{

/**
 * @brief Component tag for the generic operation catalog.
 *
 * Every operation declaration names the component it belongs to, which
 * qualifies it in diagnostics and keeps the names of different components
 * apart. 
 *
 * @see REXLIB_DECLARE_OPERATION
 */
struct ops_component
{
	static const char* get_component() noexcept
	{
		return "rex.ops";
	}
};

} // namespace ops
} // namespace rexlib
