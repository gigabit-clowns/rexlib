// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_block.hpp>

#include <hardware/caching_allocator/memory_heap.hpp>

#include "mock/mock_memory_source.hpp"
#include "test_queue.hpp"

#include <boost/intrusive/set.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>

using namespace rexlib;

namespace
{

/// Stands in for whatever the driver would have handed out.
alignas(256) std::byte storage[8192];

/// The blocks below need a heap to belong to, but never a real one.
class heap_fixture
{
public:
	heap_fixture()
		: m_alignment_expectation(
			NAMED_ALLOW_CALL(m_source, get_base_alignment())
				.RETURN(256)
		)
		, m_allocate_expectation(
			NAMED_ALLOW_CALL(m_source, allocate(trompeloeil::_))
				.RETURN(storage)
		)
		, m_deallocate_expectation(
			NAMED_ALLOW_CALL(m_source, deallocate(trompeloeil::_))
		)
		, m_heap(m_source, 8192, 256)
	{
	}

	cuda::memory_heap& get() noexcept
	{
		return m_heap;
	}

private:
	cuda::mock_memory_source m_source;

	// The heap gives its region back when it dies, so what says that is
	// expected has to still be around by then. Declared before the heap, so
	// that it is destroyed after it.
	std::unique_ptr<trompeloeil::expectation> m_alignment_expectation;
	std::unique_ptr<trompeloeil::expectation> m_allocate_expectation;
	std::unique_ptr<trompeloeil::expectation> m_deallocate_expectation;

	cuda::memory_heap m_heap;
};

} // namespace

TEST_CASE(
	"a memory_block should report the components it was built from",
	"[memory_block]"
)
{
	heap_fixture heap;
	const auto queue = cuda::make_test_queue(0);

	const cuda::memory_block block(queue, 1024, &heap.get(), 256);

	CHECK( block.get_queue() == queue );
	CHECK( block.get_size() == 1024 );
	CHECK( block.get_heap() == &heap.get() );
	CHECK( block.get_offset() == 256 );
}

TEST_CASE(
	"a memory_block should point past its heap's base by its offset",
	"[memory_block]"
)
{
	heap_fixture heap;

	const cuda::memory_block block(
		cuda::queue_handle(), 1024, &heap.get(), 768
	);

	const auto base = reinterpret_cast<std::uintptr_t>(heap.get().get_data());
	const auto data = reinterpret_cast<std::uintptr_t>(block.get_data());
	CHECK( data == base + 768 );
}

TEST_CASE(
	"a memory_block should be re-homed, resized and moved within its heap",
	"[memory_block]"
)
{
	heap_fixture heap;
	cuda::memory_block block(cuda::queue_handle(), 1024, &heap.get(), 0);

	// Splitting a block and taking one over from another queue both rewrite it
	// in place rather than building a new one, so all three have to be
	// settable.
	const auto queue = cuda::make_test_queue(1);
	block.set_queue(queue);
	block.set_size(512);
	block.set_offset(256);

	CHECK( block.get_queue() == queue );
	CHECK( block.get_size() == 512 );
	CHECK( block.get_offset() == 256 );
	CHECK( block.get_heap() == &heap.get() );
}

TEST_CASE(
	"a memory_block should be free exactly while it sits in the free set",
	"[memory_block]"
)
{
	struct by_size
	{
		bool operator()(
			const cuda::memory_block &lhs,
			const cuda::memory_block &rhs
		) const noexcept
		{
			return lhs.get_size() < rhs.get_size();
		}
	};

	using free_block_set_type = boost::intrusive::multiset<
		cuda::memory_block,
		boost::intrusive::member_hook<
			cuda::memory_block,
			cuda::memory_block::free_block_set_hook_type,
			&cuda::memory_block::free_block_set_hook
		>,
		boost::intrusive::compare<by_size>
	>;

	heap_fixture heap;
	cuda::memory_block block(cuda::queue_handle(), 1024, &heap.get(), 0);

	// Being free is not a flag that can drift out of step with where the block
	// actually is; it is that membership.
	CHECK_FALSE( block.is_free() );

	free_block_set_type free_blocks;
	free_blocks.insert(block);
	CHECK( block.is_free() );

	free_blocks.erase(free_blocks.iterator_to(block));
	CHECK_FALSE( block.is_free() );
}

TEST_CASE(
	"memory_block equality should be identity",
	"[memory_block]"
)
{
	heap_fixture heap;

	const cuda::memory_block block(cuda::queue_handle(), 1024, &heap.get(), 0);
	const cuda::memory_block same_span(
		cuda::queue_handle(), 1024, &heap.get(), 0
	);

	// Two blocks never describe the same span, so comparing their fields would
	// only ever hide a bug.
	CHECK( block == block );
	CHECK( block != same_span );
}
