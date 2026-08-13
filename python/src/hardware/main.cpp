// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "command_queue.hpp"
#include "device.hpp"
#include "device_backend.hpp"
#include "device_context.hpp"
#include "device_index.hpp"
#include "device_manager.hpp"
#include "device_properties.hpp"
#include "device_session.hpp"
#include "device_type.hpp"
#include "event.hpp"
#include "event_usage_flags.hpp"
#include "memory_allocator.hpp"
#include "memory_resource.hpp"
#include "memory_resource_affinity.hpp"
#include "memory_resource_kind.hpp"

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_hardware(pybind11::module_ &m)
{
	auto command_queue = declare_command_queue(m);
	auto device = declare_device(m);
	auto device_backend = declare_device_backend(m);
	auto device_context = declare_device_context(m);
	auto device_index = declare_device_index(m);
	auto device_manager = declare_device_manager(m);
	auto device_properties = declare_device_properties(m);
	auto device_session = declare_device_session(m);
	auto device_type = declare_device_type(m);
	auto event = declare_event(m);
	auto event_usage_flag_bits = declare_event_usage_flag_bits(m);
	auto event_usage_flags = declare_event_usage_flags(m);
	auto memory_allocator = declare_memory_allocator(m);
	auto memory_resource = declare_memory_resource(m);
	auto memory_resource_affinity = declare_memory_resource_affinity(m);
	auto memory_resource_kind = declare_memory_resource_kind(m);

	define_command_queue(command_queue);
	define_device(device);
	define_device_backend(device_backend);
	define_device_context(device_context);
	define_device_index(device_index);
	define_device_manager(device_manager, m);
	define_device_properties(device_properties);
	define_device_session(device_session);
	define_device_type(device_type);
	define_event(event);
	define_event_usage_flag_bits(event_usage_flag_bits);
	define_event_usage_flags(event_usage_flags);
	define_memory_allocator(memory_allocator);
	define_memory_resource(memory_resource, m);
	define_memory_resource_affinity(memory_resource_affinity);
	define_memory_resource_kind(memory_resource_kind);
}

} // namespace hardware
} // namespace xmipp4
