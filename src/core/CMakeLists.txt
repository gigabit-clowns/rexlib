target_sources(
	rexlib-core-obj
	PRIVATE
		library_version.cpp
		plugin_loader.cpp
		plugin_manager.cpp
		service_catalog.cpp
)

add_subdirectory(dispatch)
add_subdirectory(hardware)
add_subdirectory(layout)
add_subdirectory(memory)
add_subdirectory(ndarray)
add_subdirectory(numerical)
add_subdirectory(system)
