# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	_binding
	PRIVATE
		command_queue.cpp
		device.cpp
		device_backend.cpp
		device_context.cpp
		device_index.cpp
		device_manager.cpp
		device_properties.cpp
		device_session.cpp
		device_type.cpp
		main.cpp
		memory_allocator.cpp
		memory_resource.cpp
		memory_resource_affinity.cpp
		memory_resource_kind.cpp
)
