// SPDX-License-Identifier: GPL-3.0-only

#include "parallel_for.hpp"

#include <memory>

namespace rex
{
namespace cpu
{
namespace detail
{

/**
 * @brief Recover a loop body from its type-erased context and invoke it.
 */
template <typename Body>
void invoke_parallel_body(
	const void *context,
	std::size_t begin,
	std::size_t end
)
{
	(*static_cast<const Body*>(context))(begin, end);
}

} // namespace detail

template <typename Body>
inline
void parallel_for(
	thread_pool &pool,
	std::size_t count,
	std::size_t grain_size,
	const Body &body
)
{
	pool.run(
		count,
		grain_size,
		&detail::invoke_parallel_body<Body>,
		std::addressof(body)
	);
}

} // namespace cpu
} // namespace rex
