// SPDX-License-Identifier: GPL-3.0-only

#include "slice_insertion_tuning.hpp"

namespace rexlib
{
namespace cpu
{

slice_insertion_tuning::slice_insertion_tuning() noexcept
	: slice_insertion_tuning(1 << 20, 8)
{
}

slice_insertion_tuning::slice_insertion_tuning(
	std::size_t tile_byte_budget,
	std::size_t tiles_per_thread
) noexcept
	: m_tile_byte_budget(tile_byte_budget)
	, m_tiles_per_thread(tiles_per_thread)
{
}

std::size_t slice_insertion_tuning::get_tile_byte_budget() const noexcept
{
	return m_tile_byte_budget;
}

std::size_t slice_insertion_tuning::get_tiles_per_thread() const noexcept
{
	return m_tiles_per_thread;
}

} // namespace cpu
} // namespace rexlib
