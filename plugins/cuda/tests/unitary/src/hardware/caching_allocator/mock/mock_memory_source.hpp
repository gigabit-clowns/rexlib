// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/caching_allocator/memory_source.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cuda
{

class mock_memory_source final
	: public memory_source
{
public:
	MAKE_MOCK1(allocate, void*(std::size_t), override);
	MAKE_MOCK1(deallocate, void(void*), noexcept override);
	MAKE_CONST_MOCK0(is_host_accessible, bool(), noexcept override);
	MAKE_CONST_MOCK0(get_base_alignment, std::size_t(), noexcept override);
};

} // namespace cuda
} // namespace rexlib
