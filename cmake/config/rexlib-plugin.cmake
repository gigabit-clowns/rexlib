# Helpers for building rexlib plugins. Included both from rexlib's own
# build and from the installed package config, so that an in-tree
# superbuild and a find_package() consumer see the same contract.

# Installs a plugin module where rexlib's plugin manager looks for it.
#
# Plugins are discovered in a directory named "rexlib-plugins" beside the
# rexlib shared library, whatever the prefix. That holds for a standalone
# install and for a Python wheel that nests the prefix inside the package,
# so the relative path from a plugin to the library is always the same and
# a single RPATH serves both.
function(rexlib_install_plugin TARGET)
	if(APPLE)
		set(RPATH "@loader_path/..")
	elseif(UNIX)
		set(RPATH "$ORIGIN/..")
	else()
		set(RPATH "")
	endif()

	if(RPATH)
		set_property(
			TARGET ${TARGET}
			APPEND PROPERTY INSTALL_RPATH "${RPATH}"
		)
	endif()

	install(
		TARGETS ${TARGET}
		LIBRARY DESTINATION ${REXLIB_PLUGINS_INSTALL_DIR}
		RUNTIME DESTINATION ${REXLIB_PLUGINS_INSTALL_DIR}
	)
endfunction()
