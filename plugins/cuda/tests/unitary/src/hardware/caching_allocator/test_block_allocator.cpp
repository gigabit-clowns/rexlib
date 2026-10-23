// SPDX-License-Identifier: GPL-3.0-only

#include "test_block_allocator.hpp"

#include "test_memory_source_reference.hpp"

#include <hardware/caching_allocator/memory_block.hpp>

#include <rexlib/core/span.hpp>

#include <catch2/catch_test_macros.hpp>

#include <exception>
#include <utility>

namespace rexlib
{
namespace cuda
{

block_allocator_fixture::block_allocator_fixture(
	std::size_t capacity,
	bool host_accessible
)
	: m_arena(capacity, 256)
{
	m_expectations.push_back(
		NAMED_ALLOW_CALL(m_source, get_base_alignment())
			.LR_RETURN(m_arena.get_alignment())
	);
	m_expectations.push_back(
		NAMED_ALLOW_CALL(m_source, is_host_accessible())
			.RETURN(host_accessible)
	);
	m_expectations.push_back(
		NAMED_ALLOW_CALL(m_source, allocate(trompeloeil::_))
			.LR_RETURN(m_arena.take(_1))
	);
	m_expectations.push_back(
		NAMED_ALLOW_CALL(m_source, deallocate(trompeloeil::_))
			.LR_SIDE_EFFECT(m_arena.give_back(_1))
	);

	for (auto &timeline : m_timelines)
	{
		timeline = std::make_shared<mock_command_timeline>();
		m_expectations.push_back(
			NAMED_ALLOW_CALL(*timeline, is_complete(trompeloeil::_))
				.RETURN(false)
		);
		m_expectations.push_back(
			NAMED_ALLOW_CALL(*timeline, wait(trompeloeil::_))
		);
	}

	m_allocator = std::make_shared<memory_block_allocator>(
		std::make_unique<memory_source_reference>(m_source)
	);
}

block_allocator_fixture::~block_allocator_fixture()
{
	// The allocator has to be gone before the mocks it reaches out to, since
	// it waits for every command and gives every heap back on the way out.
	m_allocator.reset();

	// A destroyed allocator owes the driver nothing. Not while an exception
	// is on its way out: that failure is the one to report.
	if (std::uncaught_exceptions() == 0)
	{
		CHECK( m_arena.get_region_count() == 0 );
	}
}

memory_block_allocator& block_allocator_fixture::get() noexcept
{
	return *m_allocator;
}

const std::shared_ptr<memory_block_allocator>&
block_allocator_fixture::get_shared() noexcept
{
	return m_allocator;
}

mock_memory_source& block_allocator_fixture::get_source() noexcept
{
	return m_source;
}

test_arena& block_allocator_fixture::get_arena() noexcept
{
	return m_arena;
}

const std::shared_ptr<mock_command_timeline>&
block_allocator_fixture::get_timeline(std::size_t index) const noexcept
{
	return m_timelines[index];
}

void* block_allocator_fixture::allocate_and_drop(
	std::size_t size,
	const command_token &pending
)
{
	auto &block = m_allocator->allocate(
		size,
		256,
		pending.get_timeline().get()
	);
	auto *result = block.get_data();

	if (pending.is_empty())
	{
		m_allocator->recycle(block, span<const command_token>());
	}
	else
	{
		m_allocator->recycle(block, make_span(&pending, 1));
	}

	return result;
}

} // namespace cuda
} // namespace rexlib
