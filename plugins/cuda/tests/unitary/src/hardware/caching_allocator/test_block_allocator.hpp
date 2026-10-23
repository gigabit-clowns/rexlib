// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../mock/mock_command_timeline.hpp"
#include "mock/mock_memory_source.hpp"
#include "test_arena.hpp"

#include <hardware/caching_allocator/memory_block_allocator.hpp>

#include <rexlib/core/hardware/command_token.hpp>

#include <array>
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
 * the device has left and which commands are still running, and can ask what
 * was demanded of either.
 *
 * Taking memory and giving it back are allowed throughout, since almost every
 * test causes some of both; a test that is about those calls says so itself,
 * which takes precedence.
 *
 * It also holds a few timelines for the tokens a test hands out. A command of
 * theirs is still running unless the test says otherwise, and can always be
 * waited for, since the allocator waits for every one of them on its way out.
 *
 * On the way out it checks that the allocator ended up owing nothing: every
 * region taken from the arena given back. A test that leaks one fails without
 * having to say so.
 */
class block_allocator_fixture
{
public:
	/// How many timelines the fixture holds.
	static constexpr std::size_t timeline_count = 3;

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
	test_arena& get_arena() noexcept;

	/**
	 * @brief Get one of the timelines.
	 *
	 * @param index Which one. Must be less than @ref timeline_count.
	 * @return const std::shared_ptr<mock_command_timeline>& The timeline.
	 */
	const std::shared_ptr<mock_command_timeline>&
	get_timeline(std::size_t index) const noexcept;

	/**
	 * @brief Allocate a block and give it straight back.
	 *
	 * What a buffer does when it is dropped while one command still uses it.
	 *
	 * @param size Size of the block, in bytes.
	 * @param pending The token of that command, whose timeline the block is
	 * also asked for. Empty if nothing uses the block.
	 * @return void* Where the block was.
	 */
	void* allocate_and_drop(std::size_t size, const command_token &pending);

private:
	mock_memory_source m_source;
	test_arena m_arena;
	std::array<
		std::shared_ptr<mock_command_timeline>,
		timeline_count
	> m_timelines;
	std::vector<std::unique_ptr<trompeloeil::expectation>> m_expectations;
	std::shared_ptr<memory_block_allocator> m_allocator;
};

} // namespace cuda
} // namespace rexlib
