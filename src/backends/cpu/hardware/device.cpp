// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/backends/cpu/device.hpp>

#include "command_queue.hpp"
#include "event.hpp"

#include <xmipp4/backends/cpu/thread_pool.hpp>

#include <core/hardware/host_memory/host_memory_resource.hpp>

#include <utility>

namespace xmipp4
{
namespace cpu
{

device::device(std::shared_ptr<thread_pool> pool)
	: m_pool(
		pool
			? std::move(pool)
			: std::make_shared<thread_pool>(
				thread_pool::get_default_worker_count()
			)
	)
{
}

device::~device() = default;

const memory_resource&
device::get_memory_resource(memory_resource_affinity /*affinity*/) const
{
	return host_memory_resource::get();
}

std::shared_ptr<xmipp4::command_queue> device::create_command_queue() const
{
	// A fresh queue every time, all of them over the one pool this device
	// owns. The only state a queue holds is that pool, so two of them are
	// interchangeable, and handing out a new one costs an allocation rather
	// than the threads.
	return std::make_shared<command_queue>(m_pool);
}

std::shared_ptr<xmipp4::event>
device::create_event(event_usage_flags /*usage*/) const
{
	return std::make_shared<event>();
}

const std::shared_ptr<thread_pool>& device::get_thread_pool() const noexcept
{
	return m_pool;
}

} // namespace cpu
} // namespace xmipp4
