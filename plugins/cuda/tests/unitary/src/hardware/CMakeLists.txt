# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cuda-unitary-tests
	PRIVATE
		test_cuda_command_queue.cpp
		test_cuda_device.cpp
		test_cuda_device_backend.cpp
		test_device_memory_resource.cpp
		test_driver.cpp
		test_error.cpp
		test_event.cpp
		test_pinned_memory_resource.cpp
		test_pooled_event_recorder.cpp
		test_stream_blocker.cpp
		test_stream_probe.cpp
		test_stream_timeline.cpp
)
add_subdirectory(caching_allocator)
