// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/ndarray/access_hazard_tracker.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{

namespace
{

bool is_write(const access_flags &access)
{
	const auto reads = access.contains(access_flag_bits::read);
	const auto writes = access.contains(access_flag_bits::write);
	if (!reads && !writes)
	{
		throw std::invalid_argument(
			"access_hazard_tracker: The access neither reads nor writes."
		);
	}

	return writes;
}

} // anonymous namespace

access_hazard_tracker::access_hazard_tracker() = default;

access_hazard_tracker::~access_hazard_tracker() = default;

void access_hazard_tracker::add(command_token token, access_flags access)
{
	const auto write = is_write(access);

	const std::lock_guard<std::mutex> lock(m_mutex);
	if (write)
	{
		m_reads.clear();
		m_write = std::move(token);
	}
	else if (!token.is_empty())
	{
		if (m_reads.size() == m_reads.capacity())
		{
			drop_complete_reads();
		}

		m_reads.push_back(std::move(token));
	}
}

void access_hazard_tracker::collect(
	access_flags access,
	std::vector<command_token> &tokens
) const
{
	const auto write = is_write(access);

	const std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_write.is_empty())
	{
		tokens.push_back(m_write);
	}

	if (write)
	{
		tokens.insert(tokens.end(), m_reads.cbegin(), m_reads.cend());
	}
}

void access_hazard_tracker::wait(access_flags access) const
{
	std::vector<command_token> tokens;
	collect(access, tokens);

	for (const auto &token : tokens)
	{
		token.wait();
	}
}

void access_hazard_tracker::drop_complete_reads()
{
	std::vector<command_token> pending;
	pending.reserve(m_reads.size());
	for (const auto &token : m_reads)
	{
		if (!token.is_complete())
		{
			pending.push_back(token);
		}
	}

	// Room for as many again, so that the reads are not looked at on every
	// call when few of them complete.
	pending.reserve(2 * pending.size());
	m_reads.swap(pending);
}

} // namespace rexlib
