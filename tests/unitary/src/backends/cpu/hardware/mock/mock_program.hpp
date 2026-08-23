// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/backends/cpu/program.hpp>
#include <xmipp4/backends/cpu/thread_pool.hpp>

#include <trompeloeil.hpp>

namespace rex
{
namespace cpu
{

class mock_program final
	: public program
{
public:
	MAKE_CONST_MOCK4(
		execute,
		void(
			span<const std::shared_ptr<buffer>> outputs,
			span<const std::shared_ptr<const buffer>> inputs,
			span<const std::shared_ptr<buffer>> scratch,
			thread_pool &pool
		),
		override
	);
};

} // namespace cpu
} // namespace rex
