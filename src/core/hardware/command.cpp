// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/command.hpp>

#include <utility>

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
command::bind_outputs(std::vector<std::shared_ptr<buffer>> outputs) noexcept
{
	m_outputs = std::move(outputs);
	return *this;
}

command&
command::bind_inputs(
	std::vector<std::shared_ptr<const buffer>> inputs
) noexcept
{
	m_inputs = std::move(inputs);
	return *this;
}

command&
command::bind_scratch(std::vector<std::shared_ptr<buffer>> scratch) noexcept
{
	m_scratch = std::move(scratch);
	return *this;
}

command&
command::bind_dependencies(std::vector<command_token> dependencies) noexcept
{
	m_dependencies = std::move(dependencies);
	return *this;
}

const std::shared_ptr<const program>& command::get_program() const noexcept
{
	return m_program;
}

span<const std::shared_ptr<buffer>> command::get_outputs() const noexcept
{
	return make_span(m_outputs);
}

span<const std::shared_ptr<const buffer>> command::get_inputs() const noexcept
{
	return make_span(m_inputs);
}

span<const std::shared_ptr<buffer>> command::get_scratch() const noexcept
{
	return make_span(m_scratch);
}

span<const command_token> command::get_dependencies() const noexcept
{
	return make_span(m_dependencies);
}

} // namespace rexlib
