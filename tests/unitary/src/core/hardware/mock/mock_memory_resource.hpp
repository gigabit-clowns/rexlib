// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/memory_resource.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_memory_resource final
	: public memory_resource
{
public:
	MAKE_CONST_MOCK0(get_kind, memory_resource_kind(), noexcept override);
	MAKE_CONST_MOCK0(
		create_allocator, 
		std::shared_ptr<memory_allocator>(), 
		override
	);
};

} // namespace rexlib
