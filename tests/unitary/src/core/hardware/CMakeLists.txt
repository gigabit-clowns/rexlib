target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_command.cpp
		test_device_context.cpp
		test_device_index.cpp
		test_device_manager.cpp
		test_device_properties.cpp
		test_device_session.cpp
		test_device_type.cpp
		test_memory_allocator_table.cpp
		test_memory_resource_kind.cpp
		test_program.cpp
		test_program_scratch_requirement.cpp
)

add_subdirectory(host_memory)
