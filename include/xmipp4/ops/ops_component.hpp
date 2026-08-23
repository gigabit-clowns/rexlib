// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rex
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
 * @see REX_DECLARE_OPERATION
 */
struct ops_component
{
	static const char* get_component() noexcept
	{
		return "xmipp4.ops";
	}
};

} // namespace ops
} // namespace rex
