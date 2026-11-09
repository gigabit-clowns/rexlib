// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_compatibility_check.hpp"

#include <rexlib/core/exceptions/plugin_load_error.hpp>
#include <rexlib/core/version.hpp>

#include <cstddef>
#include <sstream>
#include <string>

namespace rexlib
{

namespace
{

// The record up to its last field when rexlib first published it.
const std::size_t first_record_size =
	offsetof(plugin_compatibility, runtime) + sizeof(std::uint32_t);

version get_rexlib_version(const plugin_compatibility& record)
{
	return version(
		record.rexlib_version_major,
		record.rexlib_version_minor,
		record.rexlib_version_patch
	);
}

bool is_version_compatible(
	const plugin_compatibility& plugin,
	const plugin_compatibility& library
)
{
	if (plugin.rexlib_version_major != library.rexlib_version_major)
	{
		return false;
	}
	if (library.rexlib_version_major == 0)
	{
		return plugin.rexlib_version_minor == library.rexlib_version_minor;
	}
	return plugin.rexlib_version_minor <= library.rexlib_version_minor;
}

bool is_abi_compatible(
	const plugin_compatibility& plugin,
	const plugin_compatibility& library
)
{
	return plugin.abi_family == library.abi_family &&
		plugin.standard_library == library.standard_library &&
		plugin.standard_library_abi == library.standard_library_abi &&
		plugin.runtime == library.runtime;
}

std::string describe_abi(const plugin_compatibility& record)
{
	std::ostringstream description;

	switch (record.abi_family)
	{
	case REXLIB_CPP_ABI_ITANIUM:
		description << "the Itanium C++ ABI";
		break;
	case REXLIB_CPP_ABI_MSVC:
		description << "the MSVC C++ ABI";
		break;
	default:
		description << "C++ ABI " << record.abi_family;
		break;
	}

	description << " with ";
	switch (record.standard_library)
	{
	case REXLIB_CPP_LIBSTDCXX:
		description << "libstdc++ (_GLIBCXX_USE_CXX11_ABI="
			<< record.standard_library_abi << ")";
		break;
	case REXLIB_CPP_LIBCXX:
		description << "libc++ (_LIBCPP_ABI_VERSION="
			<< record.standard_library_abi << ")";
		break;
	case REXLIB_CPP_MSVC_STL:
		description << "the MSVC standard library (_ITERATOR_DEBUG_LEVEL="
			<< record.standard_library_abi << ")";
		break;
	default:
		description << "an unknown standard library";
		break;
	}

	switch (record.runtime)
	{
	case REXLIB_CPP_NO_MSVC_RUNTIME:
		break;
	case REXLIB_CPP_MSVC_STATIC_RUNTIME:
		description << " and the static release runtime (/MT)";
		break;
	case REXLIB_CPP_MSVC_STATIC_DEBUG_RUNTIME:
		description << " and the static debug runtime (/MTd)";
		break;
	case REXLIB_CPP_MSVC_DYNAMIC_RUNTIME:
		description << " and the dynamic release runtime (/MD)";
		break;
	case REXLIB_CPP_MSVC_DYNAMIC_DEBUG_RUNTIME:
		description << " and the dynamic debug runtime (/MDd)";
		break;
	default:
		description << " and runtime " << record.runtime;
		break;
	}

	return description.str();
}

} // anonymous namespace

void check_plugin_compatibility(
	const plugin_compatibility& plugin,
	const plugin_compatibility& library
)
{
	if (plugin.size < first_record_size)
	{
		throw plugin_load_error(
			"The compatibility record of the plugin holds " +
			std::to_string(plugin.size) + " bytes, fewer than the " +
			std::to_string(first_record_size) + " every plugin states."
		);
	}

	if (!is_version_compatible(plugin, library))
	{
		std::ostringstream message;
		message << "The plugin was built against rexlib "
			<< get_rexlib_version(plugin) << ", but rexlib "
			<< get_rexlib_version(library) << " is running; ";
		if (library.rexlib_version_major == 0)
		{
			message << "before 1.0 a plugin must be built against the "
				"minor version it runs with.";
		}
		else
		{
			message << "a plugin must be built against the major version "
				"it runs with, and no newer a minor version.";
		}
		throw plugin_load_error(message.str());
	}

	if (!is_abi_compatible(plugin, library))
	{
		throw plugin_load_error(
			"The plugin was built for " + describe_abi(plugin) +
			", but rexlib was built for " + describe_abi(library) + "."
		);
	}
}

} // namespace rexlib
