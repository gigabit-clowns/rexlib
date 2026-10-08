// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/command_token.hpp>

#include <rexlib/core/hardware/command_timeline.hpp>

#include <utility>

namespace rexlib
{

command_token::command_token() noexcept
	: m_id(0)
{
}

command_token::command_token(
	std::shared_ptr<const command_timeline> timeline,
	std::size_t id
) noexcept
	: m_timeline(std::move(timeline))
	, m_id(id)
{
}

command_token::command_token(const command_token &other) noexcept = default;

command_token::command_token(command_token &&other) noexcept = default;

command_token::~command_token() = default;

command_token&
command_token::operator=(const command_token &other) noexcept = default;

command_token&
command_token::operator=(command_token &&other) noexcept = default;

const std::shared_ptr<const command_timeline>&
command_token::get_timeline() const noexcept
{
	return m_timeline;
}

std::size_t command_token::get_id() const noexcept
{
	return m_id;
}

bool command_token::is_complete() const
{
	return !m_timeline || m_timeline->is_complete(m_id);
}

void command_token::wait() const
{
	if (m_timeline)
	{
		m_timeline->wait(m_id);
	}
}

} // namespace rexlib
