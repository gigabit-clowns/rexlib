// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <catch2/catch_test_macros.hpp>

#include <rex/core/service_catalog.hpp>
#include <rex/core/dispatch/execution_context.hpp>
#include <rex/core/dispatch/program_manager.hpp>
#include <rex/core/dispatch/dispatcher.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/hardware/device_manager.hpp>
#include <rex/core/hardware/device_index.hpp>
#include <rex/core/hardware/device_context.hpp>
#include <rex/core/hardware/buffer.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rex
{

class cpu_execution_context_fixture
{
public:
	cpu_execution_context_fixture()
	{
		const auto device_manager =
			catalog.get_service_manager<xmipp4::device_manager>();
		const auto session = device_manager->create_device_session(
			device_index("cpu", 0)
		);
		const auto program_manager =
			catalog.get_service_manager<xmipp4::program_manager>();
		context = execution_context(
			device_context(session),
			make_eager_dispatcher(program_manager)
		);
	}

protected:
	array_descriptor make_descriptor(
		std::vector<std::size_t> extents = { 2, 3 },
		numerical_type data_type = numerical_type::float32
	) const
	{
		return array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type
		);
	}

	// Reads back the first count elements of a host-accessible array as type T.
	template <typename T>
	std::vector<T> read_host(const array &arr, std::size_t count) const
	{
		const auto *storage = arr.get_storage();
		REQUIRE( storage != nullptr );

		const auto *data = static_cast<const T*>(storage->get_host_ptr());
		REQUIRE( data != nullptr );

		return std::vector<T>(data, data + count);
	}

	service_catalog catalog;
	execution_context context;
};

} // namespace rex
