# SPDX-License-Identifier: GPL-3.0-only

add_library(plugin-with-missing-symbol MODULE plugin_with_missing_symbol.cpp)

# The linker of macOS refuses a symbol it cannot resolve unless told to
# leave it to the loader.
if(APPLE)
	target_link_options(
		plugin-with-missing-symbol
		PRIVATE
			LINKER:-undefined,dynamic_lookup
	)
endif()
