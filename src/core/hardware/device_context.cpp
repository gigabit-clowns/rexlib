// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/hardware/device_context.hpp>

#include <rex/core/hardware/device_session.hpp>
#include <rex/core/hardware/device.hpp>

#include <stdexcept>
#include <utility>

namespace rex
{

device_context::device_context(std::shared_ptr<const device_session> session)
	: m_session(std::move(session))
{
	if (!m_session)
	{
		throw std::invalid_argument(
			"Cannot construct a device_context from a null device_session."
		);
	}

	m_active_queue = m_session->get_default_queue();

	for (std::size_t i = 0; i < m_allocators.size(); ++i)
	{
		const auto affinity = static_cast<memory_resource_affinity>(i);
		m_allocators[i] = m_session->get_allocator(affinity);
	}
}

const std::shared_ptr<const device_session>&
device_context::get_device_session() const noexcept
{
	return m_session;
}

const std::shared_ptr<command_queue>&
device_context::get_active_queue() const noexcept
{
	return m_active_queue;
}

const std::shared_ptr<memory_allocator>&
device_context::get_allocator(memory_resource_affinity affinity) const noexcept
{
	return m_allocators[static_cast<std::size_t>(affinity)];
}

device_context
device_context::on_queue(std::shared_ptr<command_queue> queue) const
{
	if (!queue && m_session)
	{
		queue = m_session->get_default_queue();
	}

	auto result = *this;
	result.m_active_queue = std::move(queue);
	return result;
}

device_context
device_context::with_allocator(
	memory_resource_affinity affinity,
	std::shared_ptr<memory_allocator> allocator
) const
{
	if (!allocator && m_session)
	{
		allocator = m_session->get_allocator(affinity);
	}

	auto result = *this;
	const auto index = static_cast<std::size_t>(affinity);
	result.m_allocators[index] = std::move(allocator);
	return result;
}

} // namespace rex
