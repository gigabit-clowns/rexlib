// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_token.hpp>

#include <cstddef>

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>

namespace rexlib
{
namespace cuda
{

class memory_heap;

/**
 * @brief A contiguous span of a @ref memory_heap tracked by a caching pool.
 *
 * Blocks are linked into two containers of the pool at once, hence the two
 * hooks: one list of every block of every heap in address order, which is what
 * makes coalescing a neighbour lookup, and one set of the free ones only, which
 * is what makes finding a candidate a lower bound.
 *
 * A block carries the token of the last command that used its memory. Whoever
 * is handed the block next has to wait for that command, and the token is what
 * lets it do so. A block that nothing uses carries an empty token.
 */
class memory_block
{
public:
	using block_list_hook_type =
		boost::intrusive::list_member_hook<>;
	using free_block_set_hook_type =
		boost::intrusive::set_member_hook<>;

	block_list_hook_type block_list_hook;
	free_block_set_hook_type free_block_set_hook;

	/**
	 * @brief Construct a block from its components.
	 *
	 * @param pending Token of the last command that used the memory. Empty
	 * if nothing uses it.
	 * @param size Number of bytes spanned.
	 * @param heap Non owning pointer to the heap the span belongs to.
	 * @param offset Offset of the span into the heap, in bytes.
	 */
	memory_block(
		command_token pending,
		std::size_t size,
		memory_heap *heap,
		std::size_t offset
	) noexcept;

	memory_block(const memory_block &other) = default;
	memory_block(memory_block &&other) = default;
	~memory_block() = default;

	memory_block& operator=(const memory_block &other) = default;
	memory_block& operator=(memory_block &&other) = default;

	/**
	 * @brief Set the token of the last command that used the memory.
	 *
	 * @param pending The new token. Empty if nothing uses the memory.
	 */
	void set_pending(command_token pending) noexcept;

	/**
	 * @brief Get the token of the last command that used the memory.
	 *
	 * @return const command_token& The token. Empty if nothing uses the
	 * memory.
	 */
	const command_token& get_pending() const noexcept;

	/**
	 * @brief Set the number of bytes spanned by this block.
	 *
	 * @param size The new size, in bytes.
	 */
	void set_size(std::size_t size) noexcept;

	/**
	 * @brief Get the number of bytes spanned by this block.
	 *
	 * @return std::size_t Size, in bytes.
	 */
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get the heap this block spans part of.
	 *
	 * @return memory_heap* Non owning pointer to the heap.
	 */
	memory_heap* get_heap() const noexcept;

	/**
	 * @brief Set the offset of this block into its heap.
	 *
	 * @param offset The new offset, in bytes.
	 */
	void set_offset(std::size_t offset) noexcept;

	/**
	 * @brief Get the offset of this block into its heap.
	 *
	 * @return std::size_t Offset, in bytes.
	 */
	std::size_t get_offset() const noexcept;

	/**
	 * @brief Get the start of the memory spanned by this block.
	 *
	 * @return void* Pointer to the first byte.
	 */
	void* get_data() const noexcept;

	/**
	 * @brief Check whether this block is available to be handed out.
	 *
	 * @return true The block is free.
	 * @return false The block is in use, or waiting for work to finish before
	 * it can be reused.
	 */
	bool is_free() const noexcept;

	/// Blocks stand for a region of a heap, which no two of them share, so
	/// each one is only ever equal to itself.
	friend bool operator==(
		const memory_block &lhs,
		const memory_block &rhs
	) noexcept
	{
		return &lhs == &rhs;
	}

	friend bool operator!=(
		const memory_block &lhs,
		const memory_block &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	command_token m_pending;
	std::size_t m_size;
	memory_heap *m_heap;
	std::size_t m_offset;
};

} // namespace cuda
} // namespace rexlib

#include "memory_block.inl"
