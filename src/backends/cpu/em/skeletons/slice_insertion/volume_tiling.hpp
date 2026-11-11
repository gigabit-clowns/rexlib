// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "volume_tile.hpp"

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Choose the extent of the square tiles that cover a volume.
 *
 * The extent is the largest power of two that keeps a tile within the
 * byte budget and gives at least the task floor of tiles, and no less than
 * eight times the half-width, so that the samples straddling tile edges
 * stay few. It never exceeds the extent of the volume.
 *
 * @param extent The extent of the volume along y and z. Must be positive.
 * @param row_bytes The bytes one stored row of the volume takes, over
 * every accumulator.
 * @param byte_budget The bytes a tile may take.
 * @param task_floor The number of tiles wanted at least.
 * @param half_width The half-width of the gridding window, in voxels.
 * @return std::size_t The extent of a tile along y and z.
 */
std::size_t choose_tile_extent(
	std::size_t extent,
	std::size_t row_bytes,
	std::size_t byte_budget,
	std::size_t task_floor,
	double half_width
) noexcept;

/**
 * @brief The square tiles covering the stored rows of a volume.
 */
class volume_tiling
{
public:
	/**
	 * @brief Cover a volume with tiles.
	 *
	 * @param extent The extent of the volume along y and z. Must be
	 * positive.
	 * @param tile_extent The extent of a tile along y and z. Must be
	 * positive. The last tile along an axis is cut to the volume.
	 */
	volume_tiling(std::size_t extent, std::size_t tile_extent) noexcept;

	/**
	 * @brief Get the number of tiles.
	 *
	 * @return std::size_t The count.
	 */
	std::size_t get_tile_count() const noexcept;

	/**
	 * @brief Get a tile.
	 *
	 * @param index Index of the tile. Must be below get_tile_count().
	 * @return volume_tile The tile.
	 */
	volume_tile get_tile(std::size_t index) const noexcept;

private:
	std::size_t m_extent;
	std::size_t m_tile_extent;
	std::size_t m_tiles_per_axis;
};

} // namespace cpu
} // namespace rexlib
