// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "device_type.hpp"
#include "device_index.hpp"
#include "device_properties.hpp"
#include "memory_resource_kind.hpp"
#include "memory_resource_affinity.hpp"
#include "memory_resource.hpp"
#include "event_usage_flags.hpp"
#include "event.hpp"
#include "command_queue.hpp"
#include "memory_allocator.hpp"
#include "device.hpp"
#include "device_backend.hpp"
#include "device_manager.hpp"
#include "device_session.hpp"
#include "device_context.hpp"

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_hardware(pybind11::module_ &m)
{
	bind_device_type(m);
	bind_device_index(m);
	bind_device_properties(m);
	bind_memory_resource_kind(m);
	bind_memory_resource_affinity(m);
	bind_memory_resource(m);
	bind_event_usage_flags(m);
	bind_event(m);
	bind_command_queue(m);
	bind_memory_allocator(m);
	bind_device(m);
	bind_device_backend(m);
	bind_device_session(m);
	bind_device_manager(m);
	bind_device_context(m);
}

} // namespace hardware
} // namespace xmipp4
