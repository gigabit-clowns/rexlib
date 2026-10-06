// SPDX-License-Identifier: GPL-3.0-only

#include "image_scratch_runs.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

std::size_t
compute_run_count(std::size_t slot_count, std::size_t run_length)
{
	if (run_length == 0)
	{
		throw std::invalid_argument(
			"image_scratch_runs: A run must span at least one slot."
		);
	}

	if (slot_count == 0)
	{
		return 0;
	}

	return (slot_count - 1) / run_length + 1;
}

} // anonymous namespace

image_scratch_runs::image_scratch_runs(
	std::size_t slot_count,
	std::size_t run_length
)
	: m_slot_count(slot_count)
	, m_run_length(run_length)
{
	const auto run_count = compute_run_count(slot_count, run_length);

	m_present = std::make_unique<std::atomic<bool>[]>(run_count);
	for (std::size_t run = 0; run < run_count; ++run)
	{
		m_present[run].store(false, std::memory_order_relaxed);
	}
}

std::size_t image_scratch_runs::get_run(std::size_t slot) const noexcept
{
	REXLIB_ASSERT(slot < m_slot_count);
	return slot / m_run_length;
}

std::size_t image_scratch_runs::get_first_slot(std::size_t run) const noexcept
{
	REXLIB_ASSERT(run * m_run_length < m_slot_count);
	return run * m_run_length;
}

std::size_t image_scratch_runs::get_slot_count(std::size_t run) const noexcept
{
	return std::min(m_run_length, m_slot_count - get_first_slot(run));
}

bool image_scratch_runs::is_present(std::size_t run) const noexcept
{
	REXLIB_ASSERT(run * m_run_length < m_slot_count);
	return m_present[run].load(std::memory_order_acquire);
}

void image_scratch_runs::mark_present(std::size_t run) noexcept
{
	REXLIB_ASSERT(run * m_run_length < m_slot_count);
	m_present[run].store(true, std::memory_order_release);
}

} // namespace em
} // namespace rexlib
