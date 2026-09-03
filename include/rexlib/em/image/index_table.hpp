// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief A sequence of indices of one and the same rank.
 *
 * Every index a plan holds is a tuple of one coordinate per axis, and a plan
 * holds as many of them as it has regions. Held one vector per index that
 * would be one allocation per region; held here it is one flat vector of
 * @ref get_size by @ref get_rank values, so a batch of any size costs a
 * bounded number of allocations and @ref clear keeps the capacity: one
 * instance reused from one call to the next allocates nothing after the
 * first.
 *
 * The rank is stated when a table is constructed and never changes, so every
 * index it holds carries it. A table of rank zero holds indices that address
 * a single position, which is what a plan over a rank zero side needs.
 */
class index_table
{
public:
	/**
	 * @brief Construct an empty table of rank zero.
	 */
	REXLIB_API
	index_table() noexcept;

	/**
	 * @brief Construct an empty table of a given rank.
	 *
	 * @param rank Number of coordinates each index carries.
	 */
	REXLIB_API
	explicit index_table(std::size_t rank);

	REXLIB_API
	index_table(const index_table &other);
	REXLIB_API
	index_table(index_table &&other) noexcept;
	REXLIB_API
	~index_table();

	REXLIB_API
	index_table& operator=(const index_table &other);
	REXLIB_API
	index_table& operator=(index_table &&other) noexcept;

	/**
	 * @brief Append one index.
	 *
	 * @param index The coordinates. Their number must equal @ref get_rank.
	 * @throws std::invalid_argument If @p index does not have the rank of
	 * this table.
	 */
	REXLIB_API
	void add(span<const std::size_t> index);

	/**
	 * @brief Drop the indices held, keeping the rank and the capacity.
	 */
	REXLIB_API
	void clear() noexcept;

	/**
	 * @brief Make room for a number of indices without allocating later.
	 *
	 * @param count Number of indices to make room for.
	 */
	REXLIB_API
	void reserve(std::size_t count);

	/**
	 * @brief Get the number of coordinates each index carries.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_rank() const noexcept;

	/**
	 * @brief Get how many indices are held.
	 *
	 * @return std::size_t The number of indices.
	 */
	REXLIB_API
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get one index.
	 *
	 * @param position Position of the index. Must be below @ref get_size.
	 * @return span<const std::size_t> The coordinates, of rank
	 * @ref get_rank. It refers to storage owned by this table, which adding
	 * to, assigning to or destroying it invalidates.
	 */
	REXLIB_API
	span<const std::size_t> get(std::size_t position) const noexcept;

private:
	std::vector<std::size_t> m_values;
	std::size_t m_rank;
	std::size_t m_size;
};

} // namespace em
} // namespace rexlib
