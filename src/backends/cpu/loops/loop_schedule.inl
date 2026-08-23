// SPDX-License-Identifier: GPL-3.0-only

#include "loop_schedule.hpp"

#include <xmipp4/backends/cpu/parallel_for.hpp>

namespace rex
{
namespace cpu
{

inline
loop_schedule::loop_schedule() noexcept
	: m_pool(nullptr)
	, m_grain_size(1)
{
}

inline
loop_schedule::loop_schedule(
	thread_pool &pool,
	std::size_t grain_size
) noexcept
	: m_pool(&pool)
	, m_grain_size(grain_size)
{
}

inline
std::size_t loop_schedule::get_concurrency() const noexcept
{
	return m_pool ? m_pool->get_size() : 1;
}

inline
std::size_t loop_schedule::get_grain_size() const noexcept
{
	return m_grain_size;
}

inline
loop_schedule loop_schedule::with_grain(std::size_t grain_size) const noexcept
{
	auto result = *this;
	result.m_grain_size = grain_size;
	return result;
}

inline
loop_schedule loop_schedule::serial() const noexcept
{
	auto result = *this;
	result.m_pool = nullptr;
	return result;
}

template <typename Body>
inline
void loop_schedule::run(std::size_t count, const Body &body) const
{
	if (m_pool)
	{
		parallel_for(*m_pool, count, m_grain_size, body);
	}
	else if (count > 0)
	{
		body(0, count);
	}
}

} // namespace cpu
} // namespace rex
