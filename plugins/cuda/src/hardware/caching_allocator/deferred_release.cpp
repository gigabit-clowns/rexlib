// SPDX-License-Identifier: GPL-3.0-only

#include "deferred_release.hpp"

#include "event_recorder.hpp"
#include "memory_block.hpp"
#include "memory_block_pool.hpp"

#include "../../logger.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

deferred_release::deferred_release() noexcept = default;

deferred_release::~deferred_release()
{
	if (!m_pending.empty())
	{
		// Only reachable if the owner let this go without draining it, which
		// it has to do while it still has the pool to give the blocks back to.
		REXLIB_CUDA_LOG_ERROR(
			"A deferred_release was destroyed while still holding blocks. "
			"They are lost."
		);
	}
}

void deferred_release::defer(
	event_recorder &recorder,
	memory_block &block,
	span<const queue_handle> queues
)
{
	if (queues.empty())
	{
		throw std::invalid_argument(
			"deferred_release::defer: A block that no other queue used should "
			"be given back to the pool directly."
		);
	}

	// Captured into a local first, so that failing part way through leaves
	// nothing half waited for behind.
	std::vector<event_ticket> tickets;
	tickets.reserve(queues.size());
	for (const auto &queue : queues)
	{
		if (!queue)
		{
			throw std::invalid_argument(
				"deferred_release::defer: A block can not be waiting for a "
				"queue that does not exist."
			);
		}

		tickets.emplace_back(recorder, queue);
	}

	m_pending.emplace_back(&block, std::move(tickets));
}

void deferred_release::process(memory_block_pool &pool)
{
	// Every block is looked at before any of them is moved. Compacting as we
	// go would mean a failing query leaves entries behind that have been moved
	// out of, and an entry with no points left to wait for is exactly what a
	// block that is ready to be handed out again looks like.
	for (auto &item : m_pending)
	{
		drop_reached_points(item);
	}

	const auto last = std::remove_if(
		m_pending.begin(), m_pending.end(),
		[&pool] (const pending_release &item) noexcept
		{
			const auto reached = item.second.empty();
			if (reached)
			{
				pool.release(*item.first);
			}
			return reached;
		}
	);

	m_pending.erase(last, m_pending.end());
}

void deferred_release::wait_all(memory_block_pool &pool)
{
	// Each block leaves the list as it goes back, so a wait failing part way
	// through cannot have the next drain give the earlier ones back twice.
	auto ite = m_pending.begin();
	while (ite != m_pending.end())
	{
		for (const auto &ticket : ite->second)
		{
			ticket.wait();
		}

		pool.release(*ite->first);
		ite = m_pending.erase(ite);
	}
}

std::size_t deferred_release::get_pending_count() const noexcept
{
	return m_pending.size();
}

void deferred_release::drop_reached_points(pending_release &item)
{
	auto &tickets = item.second;

	// Same reason as in process: every point is queried before any ticket is
	// moved, so that a query failing leaves the block waiting for exactly the
	// points it was waiting for, rather than for a shuffled subset of them.
	reached_point_flags reached(tickets.size());
	for (std::size_t i = 0; i < tickets.size(); ++i)
	{
		reached[i] = tickets[i].is_complete();
	}

	std::size_t kept = 0;
	for (std::size_t i = 0; i < tickets.size(); ++i)
	{
		if (!reached[i])
		{
			if (kept != i)
			{
				tickets[kept].swap(tickets[i]);
			}
			++kept;
		}
	}

	// Dropping the tickets gives them back to the recorder, which is what lets
	// the events behind them be recorded again instead of piling up.
	tickets.resize(kept);
}

} // namespace cuda
} // namespace rexlib
