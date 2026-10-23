// SPDX-License-Identifier: GPL-3.0-only

#include "deferred_release.hpp"

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
	memory_block &block,
	span<const command_token> pending
)
{
	if (pending.size() < 2)
	{
		throw std::invalid_argument(
			"deferred_release::defer: A block that at most one command still "
			"uses should be given back to the pool directly."
		);
	}

	m_pending.emplace_back(
		&block,
		std::vector<command_token>(pending.begin(), pending.end())
	);
}

void deferred_release::process(memory_block_pool &pool)
{
	// Every block is looked at before any of them is moved. Compacting as we
	// go would mean a failing query leaves entries behind that have been moved
	// out of, and an entry with a single token left is exactly what a block
	// that is ready to go back looks like.
	for (auto &item : m_pending)
	{
		drop_complete_tokens(item);
	}

	const auto last = std::remove_if(
		m_pending.begin(), m_pending.end(),
		[&pool] (const pending_release &item) noexcept
		{
			const auto &tokens = item.second;
			const auto ready = tokens.size() < 2;
			if (ready)
			{
				pool.release(
					*item.first,
					tokens.empty() ? command_token() : tokens.front()
				);
			}
			return ready;
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
		for (const auto &token : ite->second)
		{
			token.wait();
		}

		pool.release(*ite->first, command_token());
		ite = m_pending.erase(ite);
	}
}

std::size_t deferred_release::get_pending_count() const noexcept
{
	return m_pending.size();
}

void deferred_release::drop_complete_tokens(pending_release &item)
{
	auto &tokens = item.second;

	// Same reason as in process: every token is queried before any of them is
	// moved, so that a query failing leaves the block waiting for exactly the
	// tokens it was waiting for, rather than for a shuffled subset of them.
	completion_flags complete(tokens.size());
	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		complete[i] = tokens[i].is_complete();
	}

	std::size_t kept = 0;
	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		if (!complete[i])
		{
			if (kept != i)
			{
				tokens[kept] = std::move(tokens[i]);
			}
			++kept;
		}
	}

	tokens.resize(kept);
}

} // namespace cuda
} // namespace rexlib
