// SPDX-License-Identifier: GPL-3.0-only

#include "claimed_loop.hpp"

#include <rexlib/backends/cpu/parallel_for.hpp>

#include <atomic>

namespace rexlib
{
namespace cpu
{

template <typename Body>
inline
void run_claimed_loop(thread_pool &pool, std::size_t count, const Body &body)
{
	if (count == 0)
	{
		return;
	}

	std::atomic<std::size_t> next(0);
	parallel_for(
		pool,
		pool.get_size(),
		1,
		[&](std::size_t, std::size_t)
		{
			for (;;)
			{
				const auto index = next.fetch_add(1, std::memory_order_relaxed);
				if (index >= count)
				{
					return;
				}
				body(index);
			}
		}
	);
}

} // namespace cpu
} // namespace rexlib
