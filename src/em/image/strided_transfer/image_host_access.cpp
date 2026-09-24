// SPDX-License-Identifier: GPL-3.0-only

#include "image_host_access.hpp"

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

void check_storage(const buffer *storage)
{
	if (storage == nullptr)
	{
		throw std::invalid_argument(
			"get_host_data: The array is not initialized."
		);
	}

	if (!is_host_accessible(storage->get_memory_resource().get_kind()))
	{
		throw unsupported_capability_error(
			"get_host_data: The storage of the array can not be reached from "
			"the host."
		);
	}
}

void check_data(const void *data)
{
	if (data == nullptr)
	{
		throw unsupported_capability_error(
			"get_host_data: The array does not expose its storage to the "
			"host."
		);
	}
}

} // anonymous namespace

void* get_host_data(array_ref array)
{
	auto *storage = array.get_storage();
	check_storage(storage);

	auto *data = storage->get_host_ptr();
	check_data(data);

	return data;
}

const void* get_host_data(const_array_ref array)
{
	const auto *storage = array.get_storage();
	check_storage(storage);

	const auto *data = storage->get_host_ptr();
	check_data(data);

	return data;
}

} // namespace em
} // namespace rexlib
