// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/memory/aligned_alloc.hpp>

#include <rex/core/platform/operating_system.h>

#include <algorithm>
#include <cstdlib>

namespace rexlib
{

void* aligned_alloc(std::size_t size, std::size_t alignment) noexcept
{
	#if REXLIB_WINDOWS
		return _aligned_malloc(size, alignment);
	#elif REXLIB_POSIX
		void *result;
		
		alignment = std::max(alignment, sizeof(void*));
		if(posix_memalign(&result, alignment, size) != 0)
		{
			result = nullptr;
		}

		return result;
	#else
		return std::aligned_alloc(alignment, size);
	#endif
}

void aligned_free(void* ptr) noexcept
{
	#if REXLIB_WINDOWS
		_aligned_free(ptr);
	#else
		std::free(ptr);
	#endif
}

} // namespace rexlib
