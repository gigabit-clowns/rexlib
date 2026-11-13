// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/version_macros.h>

#include "platform/cpp_abi.hpp"

#include <cstdint>

namespace rexlib
{

/**
 * @brief What a plugin was built for: the version of rexlib and the C++
 * ABI.
 *
 * Plain data of a fixed layout, so that it can be read whatever the C++
 * ABI of the plugin. Fields are only ever appended, and size says how
 * many of them the plugin knows. The values are those of
 * REXLIB_PLUGIN_COMPATIBILITY.
 */
struct plugin_compatibility
{
	/** @brief The size of the record the plugin was built with. */
	std::uint32_t size;
	/** @brief The major version of rexlib the plugin was built against. */
	std::uint32_t rexlib_version_major;
	/** @brief The minor version of rexlib the plugin was built against. */
	std::uint32_t rexlib_version_minor;
	/** @brief The patch version of rexlib the plugin was built against. */
	std::uint32_t rexlib_version_patch;
	/** @brief The value of REXLIB_CPP_ABI_FAMILY. */
	std::uint32_t abi_family;
	/** @brief The value of REXLIB_CPP_STANDARD_LIBRARY. */
	std::uint32_t standard_library;
	/** @brief The value of REXLIB_CPP_STANDARD_LIBRARY_ABI. */
	std::uint32_t standard_library_abi;
	/** @brief The value of REXLIB_CPP_RUNTIME. */
	std::uint32_t runtime;
};

} // namespace rexlib

/**
 * @def REXLIB_PLUGIN_COMPATIBILITY
 * @brief The plugin_compatibility of the code being compiled.
 *
 * A braced initializer. It states the build of the code that expands it,
 * not that of the library.
 */
#define REXLIB_PLUGIN_COMPATIBILITY \
	{ \
		static_cast<std::uint32_t>(sizeof(::rexlib::plugin_compatibility)), \
		REXLIB_VERSION_MAJOR, \
		REXLIB_VERSION_MINOR, \
		REXLIB_VERSION_PATCH, \
		REXLIB_CPP_ABI_FAMILY, \
		REXLIB_CPP_STANDARD_LIBRARY, \
		REXLIB_CPP_STANDARD_LIBRARY_ABI, \
		REXLIB_CPP_RUNTIME \
	}
