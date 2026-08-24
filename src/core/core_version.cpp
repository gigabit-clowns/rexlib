// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/core_version.hpp>

namespace rexlib
{

version get_core_version() noexcept
{
	return version(
		REXLIB_VERSION_MAJOR,
		REXLIB_VERSION_MINOR,
		REXLIB_VERSION_PATCH
	);
}

} // namespace rexlib
