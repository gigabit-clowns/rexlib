# SPDX-License-Identifier: GPL-3.0-only

# Helpers for building rexlib plugins. Included both from rexlib's own
# build and from the installed package config, so that an in-tree
# superbuild and a find_package() consumer see the same contract.

# Installs a plugin module.
#
#   rexlib_install_plugin(<target> [DESTINATION <dir>])
#
# Without DESTINATION, the plugin goes where rexlib's plugin manager looks
# for it: a directory named "rexlib-plugins" beside the rexlib shared
# library, whatever the prefix. That holds for a standalone install and for
# a Python wheel that nests the prefix inside the package, so the relative
# path from a plugin to the library is always the same and a single RPATH
# serves both.
#
# With DESTINATION, a directory relative to the install prefix, the plugin
# goes there and no RPATH is added, as for a plugin wheel installed into a
# package directory of its own. Its reference to librexlib resolves to the
# copy already loaded by whatever loads the plugin.
function(rexlib_install_plugin TARGET)
	cmake_parse_arguments(PARSE_ARGV 1 arg "" "DESTINATION" "")
	if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR
			"rexlib_install_plugin takes a target and an optional "
			"DESTINATION <dir>"
		)
	endif()

	if(DEFINED arg_DESTINATION)
		set(DESTINATION "${arg_DESTINATION}")
		set(RPATH "")
	else()
		set(DESTINATION "${REXLIB_PLUGINS_INSTALL_DIR}")
		if(APPLE)
			set(RPATH "@loader_path/..")
		elseif(UNIX)
			set(RPATH "$ORIGIN/..")
		else()
			set(RPATH "")
		endif()
	endif()

	if(RPATH)
		set_property(
			TARGET ${TARGET}
			APPEND PROPERTY INSTALL_RPATH "${RPATH}"
		)
	endif()

	install(
		TARGETS ${TARGET}
		LIBRARY DESTINATION "${DESTINATION}"
		RUNTIME DESTINATION "${DESTINATION}"
	)
endfunction()
