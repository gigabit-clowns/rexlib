// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mock/mock_event_recorder.hpp"
#include "mock/mock_memory_source.hpp"
#include "test_arena.hpp"

#include <hardware/caching_allocator/memory_block_allocator.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace rexlib
{
namespace cuda
{

/**
 * @brief A block allocator over ordinary host memory.
 *
 * Everything the allocator reaches out to is mocked, so a test can say what
 * the device has left and when its queues have caught up, and can ask what was
 * demanded of either.
 *
 * Taking memory and giving it back are allowed throughout, since almost every
 * test causes some of both; a test that is about those calls says so itself,
 * which takes precedence. Capturing a point is answered with a fresh ticket
 * that starts out already reached, so that a test only has to say anything
 * when it wants work to still be running.
 *
 * On the way out it checks that the allocator ended up owing nothing: every
 * region taken from the arena given back, and every point captured given back
 * exactly once. A test that leaks either fails without having to say so.
 */
class block_allocator_fixture
{
public:
	/**
	 * @brief Build an allocator.
	 *
	 * @param capacity Number of bytes the device has.
	 * @param host_accessible Whether the host can address what it hands out.
	 */
	explicit block_allocator_fixture(
		std::size_t capacity = test_arena::unlimited,
		bool host_accessible = false
	);
	block_allocator_fixture(const block_allocator_fixture &other) = delete;
	block_allocator_fixture(block_allocator_fixture &&other) = delete;
	~block_allocator_fixture();

	block_allocator_fixture&
	operator=(const block_allocator_fixture &other) = delete;
	block_allocator_fixture&
	operator=(block_allocator_fixture &&other) = delete;

	memory_block_allocator& get() noexcept;
	const std::shared_ptr<memory_block_allocator>& get_shared() noexcept;
	mock_memory_source& get_source() noexcept;
	mock_event_recorder& get_recorder() noexcept;
	test_arena& get_arena() noexcept;

	/**
	 * @brief Allocate a block and give it straight back.
	 *
	 * What a buffer nobody keeps does.
	 *
	 * @param size Size of the block, in bytes.
	 * @param queue The queue the block is handed to.
	 * @return void* Where the block was.
	 */
	void* allocate_and_drop(std::size_t size, const queue_handle &queue);

	/**
	 * @brief Report the point a ticket stands for as reached.
	 *
	 * @param ticket The ticket.
	 */
	void reach(event_recorder::ticket ticket);

	/**
	 * @brief Say whether points start out reached.
	 *
	 * @param reached Whether a point captured from now on counts as reached
	 * straight away.
	 */
	void set_reached_by_default(bool reached) noexcept;

	/**
	 * @brief Get the queue a ticket captured a point of.
	 *
	 * @param ticket The ticket.
	 * @return const queue_handle& The queue.
	 */
	const queue_handle& get_captured_queue(
		event_recorder::ticket ticket
	) const noexcept;

	/**
	 * @brief Get how many captured points have not been given back.
	 *
	 * @return std::size_t Number of tickets still outstanding.
	 */
	std::size_t get_outstanding_ticket_count() const noexcept;

private:
	mock_memory_source m_source;
	mock_event_recorder m_recorder;
	test_arena m_arena;

	std::vector<queue_handle> m_captured_queues;
	std::vector<bool> m_reached;
	std::vector<bool> m_released;
	bool m_released_twice;
	bool m_reached_by_default;

	std::vector<std::unique_ptr<trompeloeil::expectation>> m_expectations;
	std::shared_ptr<memory_block_allocator> m_allocator;

	event_recorder::ticket capture(const queue_handle &queue);
	void track(event_recorder::ticket ticket);
	bool has_reached(event_recorder::ticket ticket);
	void mark_reached(event_recorder::ticket ticket);
	void give_back(event_recorder::ticket ticket);
};

} // namespace cuda
} // namespace rexlib
