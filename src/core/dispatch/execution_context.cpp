// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/execution_context.hpp>

#include <memory>
#include <utility>

namespace rexlib
{

execution_context::execution_context() noexcept = default;

execution_context::execution_context(
	device_context device_context,
	std::shared_ptr<dispatcher> dispatcher
)
	: m_device_context(std::move(device_context))
	, m_dispatcher(std::move(dispatcher))
{
}

execution_context::execution_context(const execution_context &other)
	= default;

execution_context::execution_context(execution_context &&other) noexcept
	= default;

execution_context::~execution_context() = default;

execution_context&
execution_context::operator=(const execution_context &other) = default;

execution_context&
execution_context::operator=(execution_context &&other) noexcept = default;

const device_context&
execution_context::get_device_context() const noexcept
{
	return m_device_context;
}

const std::shared_ptr<dispatcher>&
execution_context::get_dispatcher() const noexcept
{
	return m_dispatcher;
}

execution_context execution_context::with_device_context(
	device_context device_context
) const
{
	return execution_context(
		std::move(device_context),
		m_dispatcher
	);
}

execution_context execution_context::with_dispatcher(
	std::shared_ptr<dispatcher> dispatcher
) const
{
	return execution_context(
		m_device_context,
		std::move(dispatcher)
	);
}

} // namespace rexlib
