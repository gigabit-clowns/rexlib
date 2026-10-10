target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_service_catalog.cpp
		test_span.cpp
		test_version.cpp
)

add_subdirectory(binary)
add_subdirectory(dispatch)
add_subdirectory(hardware)
add_subdirectory(layout)
add_subdirectory(memory)
add_subdirectory(meta)
add_subdirectory(ndarray)
add_subdirectory(numerical)
add_subdirectory(system)
