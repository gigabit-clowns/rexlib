# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cuda-objects
	PRIVATE
		caching_memory_allocator.cpp
		cuda_buffer.cpp
		deferred_release.cpp
		device_memory_source.cpp
		memory_block_allocator.cpp
		memory_block_pool.cpp
		memory_heap.cpp
		memory_source.cpp
		pinned_memory_source.cpp
)
