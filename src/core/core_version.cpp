// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/core_version.hpp>

namespace rex
{

version get_core_version() noexcept
{
	return version(
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_PATCH
	);
}

} // namespace rex
