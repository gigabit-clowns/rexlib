# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-cuda-unitary-tests
	PRIVATE
		test_arena.cpp
		test_block_allocator.cpp
		test_caching_allocator.cpp
		test_caching_memory_allocator.cpp
		test_cuda_buffer.cpp
		test_deferred_release.cpp
		test_memory_block.cpp
		test_memory_block_allocator.cpp
		test_memory_block_pool.cpp
		test_memory_heap.cpp
		test_memory_source_reference.cpp
)
