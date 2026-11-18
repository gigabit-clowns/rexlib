// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "plugin.hpp"
#include "plugin_compatibility.hpp"
#include "platform/dynamic_shared_object.h"

#include <type_traits>

/**
 * @def REXLIB_DEFINE_PLUGIN
 * @brief Define the entry points through which rexlib loads a plugin.
 *
 * Expanded once, at namespace scope, in one source file of the plugin, it
 * defines two functions with C linkage. rexlib_get_plugin_compatibility
 * returns the plugin_compatibility of that source file, which the loader
 * compares with its own before it touches anything else.
 * rexlib_get_plugin constructs the plugin on its first call and returns
 * it, or returns null when the constructor throws, since no exception may
 * leave a function with C linkage.
 *
 * @param type The plugin: a default constructible class derived from
 * rexlib::plugin.
 */
#define REXLIB_DEFINE_PLUGIN(type) \
	extern "C" REXLIB_EXPORT \
	const ::rexlib::plugin_compatibility* rexlib_get_plugin_compatibility() \
	{ \
		static const ::rexlib::plugin_compatibility compatibility = \
			REXLIB_PLUGIN_COMPATIBILITY; \
		return &compatibility; \
	} \
	extern "C" REXLIB_EXPORT \
	const ::rexlib::plugin* rexlib_get_plugin() noexcept \
	{ \
		try \
		{ \
			static const type instance{}; \
			return &instance; \
		} \
		catch (...) \
		{ \
			return nullptr; \
		} \
	} \
	static_assert( \
		std::is_base_of<::rexlib::plugin, type>::value, \
		"REXLIB_DEFINE_PLUGIN takes a class derived from rexlib::plugin" \
	)
