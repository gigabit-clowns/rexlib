// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/buffer.hpp>

#include <trompeloeil.hpp>

namespace rex
{

class mock_buffer final
	: public buffer
{
public:
	MAKE_MOCK0(get_host_ptr, void*(), noexcept override);
	MAKE_CONST_MOCK0(get_host_ptr, const void*(), noexcept override);
	MAKE_CONST_MOCK0(get_size, std::size_t(), noexcept override);
	MAKE_CONST_MOCK0(
		get_memory_resource,
		const memory_resource&(),
		noexcept override
	);
};

} // namespace rex
