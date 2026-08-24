// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/memory_allocator.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_memory_allocator final
	: public memory_allocator
{
public:
	MAKE_CONST_MOCK0(
		get_memory_resource,
		const memory_resource&(),
		noexcept override
	);
	MAKE_CONST_MOCK0(
		get_max_alignment,
		std::size_t(),
		noexcept override
	);
	MAKE_MOCK3(
		allocate,
		std::shared_ptr<buffer>(std::size_t, std::size_t, command_queue*),
		override
	);
};

} // namespace rexlib
