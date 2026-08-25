// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/command.hpp>

namespace rexlib
{

command::command() noexcept = default;

command::command(std::shared_ptr<const program> program) noexcept
	: m_program(std::move(program))
{
}

command::command(const command &other) = default;

command::command(command &&other) noexcept = default;

command::~command() = default;

command& command::operator=(const command &other) = default;

command& command::operator=(command &&other) noexcept = default;

command& 
command::bind_outputs(span<const std::shared_ptr<buffer>> outputs) noexcept
{
	m_outputs = outputs;
	return *this;
}

command& 
command::bind_inputs(span<const std::shared_ptr<const buffer>> inputs) noexcept
{
	m_inputs = inputs;
	return *this;
}

command& 
command::bind_scratch(span<const std::shared_ptr<buffer>> scratch) noexcept
{
	m_scratch = scratch;
	return *this;
}

const std::shared_ptr<const program>& command::get_program() const noexcept
{
	return m_program;
}

span<const std::shared_ptr<buffer>> command::get_outputs() const noexcept
{
	return m_outputs;
}

span<const std::shared_ptr<const buffer>> command::get_inputs() const noexcept
{
	return m_inputs;
}

span<const std::shared_ptr<buffer>> command::get_scratch() const noexcept
{
	return m_scratch;
}

} // namespace rexlib
