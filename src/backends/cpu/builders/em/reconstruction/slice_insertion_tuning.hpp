// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The values that size the tiles of an insertion.
 */
class slice_insertion_tuning
{
public:
	/**
	 * @brief Construct the tuning the programs use.
	 *
	 * A tile takes up to 1 MiB of the accumulators, and there are at least 8
	 * tiles per thread.
	 */
	slice_insertion_tuning() noexcept;

	/**
	 * @brief Construct a tuning.
	 *
	 * @param tile_byte_budget The bytes a tile of the accumulators may take.
	 * @param tiles_per_thread The number of tiles wanted per thread at
	 * least.
	 */
	slice_insertion_tuning(
		std::size_t tile_byte_budget,
		std::size_t tiles_per_thread
	) noexcept;

	std::size_t get_tile_byte_budget() const noexcept;
	std::size_t get_tiles_per_thread() const noexcept;

private:
	std::size_t m_tile_byte_budget;
	std::size_t m_tiles_per_thread;
};

} // namespace cpu
} // namespace rexlib
