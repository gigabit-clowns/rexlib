# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cuda-objects
	PRIVATE
		cuda_command_queue.cpp
		cuda_device.cpp
		cuda_device_backend.cpp
		device_guard.cpp
		device_memory_resource.cpp
		error.cpp
		event.cpp
		event_recorder.cpp
		pinned_memory_resource.cpp
		pooled_event_recorder.cpp
		stream_program.cpp
		stream_timeline.cpp
)
add_subdirectory(caching_allocator)
