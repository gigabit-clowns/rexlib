# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	_binding
	PRIVATE
		plugin.cpp
		plugin_manager.cpp
		service_catalog.cpp
		version.cpp
)
add_subdirectory(dispatch)
add_subdirectory(hardware)
add_subdirectory(ndarray)
add_subdirectory(numerical)
