// SPDX-License-Identifier: GPL-3.0-only

#include "device_backend.hpp"

#include "device.hpp"
#include "error.hpp"

#include "../config.hpp"

#include <rexlib/core/hardware/device_manager.hpp>

#include <cstdlib>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

static std::string pci_id_to_string(int domain_id, int bus_id, int device_id)
{
	std::ostringstream oss;

	oss << std::hex << std::setfill('0');
	oss << std::setw(4) << domain_id << ':';
	oss << std::setw(2) << bus_id << ':';
	oss << std::setw(2) << device_id << ".0";

	return oss.str();
}

static bool get_device_count(int &count) noexcept
{
	// A system without a CUDA capable device or without a usable driver is
	// not an error: the backend simply has nothing to offer.
	return cudaGetDeviceCount(&count) == cudaSuccess;
}



std::string device_backend::get_name() const
{
	return "cuda";
}

version device_backend::get_version() const
{
	// Querying the runtime version goes through the driver, which fails when
	// it is missing or older than the runtime. The version the plugin was
	// built against is a valid answer there.
	int cuda_version;
	if (cudaRuntimeGetVersion(&cuda_version) != cudaSuccess)
	{
		cuda_version = CUDART_VERSION;
	}

	const auto major_div = std::div(cuda_version, 1000);
	const auto minor_div = std::div(major_div.rem, 10);

	return version(
		major_div.quot,
		minor_div.quot,
		minor_div.rem
	);
}

void device_backend::enumerate_devices(std::vector<std::size_t> &ids) const
{
	int count;
	if (!get_device_count(count))
	{
		count = 0;
	}

	ids.resize(count);
	std::iota(
		ids.begin(), ids.end(),
		static_cast<std::size_t>(0)
	);
}

bool device_backend::get_device_properties(
	std::size_t id,
	device_properties &desc
) const
{
	int count;
	if (!get_device_count(count))
	{
		return false;
	}

	const auto ordinal = static_cast<int>(id);
	const auto result = ordinal < count;
	if (result)
	{
		cudaDeviceProp prop;
		REXLIB_CUDA_CHECK( cudaGetDeviceProperties(&prop, ordinal) );

		const auto type =
			prop.integrated ? device_type::integrated_gpu : device_type::gpu;
		auto location = pci_id_to_string(
			prop.pciDomainID,
			prop.pciBusID,
			prop.pciDeviceID
		);

		desc.set_name(std::string(prop.name));
		desc.set_physical_location(std::move(location));
		desc.set_type(type);
		desc.set_total_memory_bytes(prop.totalGlobalMem);
		desc.set_optimal_data_alignment(REXLIB_CUDA_COALESCE_ALIGN_BYTES);
	}

	return result;
}

std::shared_ptr<rexlib::device>
device_backend::create_device(std::size_t id) const
{
	int count;
	if (!get_device_count(count))
	{
		count = 0;
	}

	const auto ordinal = static_cast<int>(id);
	if (ordinal >= count)
	{
		throw std::invalid_argument("Invalid device id");
	}

	return std::make_shared<device>(ordinal);
}

bool device_backend::register_at(rexlib::device_manager &manager)
{
	return manager.register_backend(std::make_unique<device_backend>());
}

} // namespace cuda
} // namespace rexlib
