// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief Sorted list of the indices that a scratch entry holds.
 *
 * An index selects one item along the first axis of a file, for example one
 * image of a stack. The indices are stored in ascending order. The position
 * of an index in this list is its slot.
 */
class image_scratch_slots
{
public:
	/**
	 * @brief Construct from a list of indices.
	 *
	 * @param indices The indices to hold. They must be in ascending order
	 * and must not repeat.
	 * @throws std::invalid_argument If @p indices is not strictly ascending.
	 */
	explicit image_scratch_slots(std::vector<std::size_t> indices);

	image_scratch_slots(const image_scratch_slots &other) = default;
	image_scratch_slots(image_scratch_slots &&other) noexcept = default;
	~image_scratch_slots() = default;

	image_scratch_slots&
	operator=(const image_scratch_slots &other) = default;
	image_scratch_slots&
	operator=(image_scratch_slots &&other) noexcept = default;

	/**
	 * @brief Get the number of slots.
	 *
	 * @return std::size_t The number of indices held.
	 */
	std::size_t get_count() const noexcept;

	/**
	 * @brief Get the index stored in a slot.
	 *
	 * @param slot The slot. Must be below @ref get_count.
	 * @return std::size_t The index.
	 */
	std::size_t get_index(std::size_t slot) const noexcept;

	/**
	 * @brief Find the first slot whose index is not below a given index.
	 *
	 * If @p index is held, this is its slot.
	 *
	 * @param index The index to search for.
	 * @return std::size_t The slot, or @ref get_count if all held indices
	 * are below @p index.
	 */
	std::size_t get_lower_bound(std::size_t index) const noexcept;

	/**
	 * @brief Count the held indices in a range of consecutive indices.
	 *
	 * The held indices of the range occupy consecutive slots, starting at
	 * the @ref get_lower_bound of @p first_index.
	 *
	 * @param first_index First index of the range.
	 * @param index_count Number of indices in the range.
	 * @return std::size_t The number of held indices. It equals
	 * @p index_count if the whole range is held.
	 */
	std::size_t count_held(
		std::size_t first_index,
		std::size_t index_count
	) const noexcept;

private:
	std::vector<std::size_t> m_indices;
};

} // namespace em
} // namespace rexlib
