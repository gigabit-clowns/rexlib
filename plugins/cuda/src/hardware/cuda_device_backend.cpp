// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_device_backend.hpp"

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/cuda/hardware/cuda_device.hpp>

#include <xmipp4/core/hardware/device_manager.hpp>

#include <numeric>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace hardware
{

static std::string pci_id_to_string(int bus_id, int device_id, int domain_id)
{
	std::ostringstream oss;

	oss << std::setfill('0') << std::setw(2) << bus_id << ':';
	oss << std::setfill('0') << std::setw(2) << device_id << '.';
	oss << std::setw(1) << domain_id << '.';

	return oss.str();
}



std::string cuda_device_backend::get_name() const
{
	return "cuda";
}

version cuda_device_backend::get_version() const
{
	int cuda_version;
	XMIPP4_CUDA_CHECK( cudaRuntimeGetVersion(&cuda_version) );

	const auto major_div = std::div(cuda_version, 1000);
	const auto minor_div = std::div(major_div.rem, 10);

	return version(
		major_div.quot,
		minor_div.quot,
		minor_div.rem
	);
}

void cuda_device_backend::enumerate_devices(std::vector<std::size_t> &ids) const
{
	int count;
	cudaGetDeviceCount(&count);
	
	ids.resize(count);
	std::iota(
		ids.begin(), ids.end(),
		static_cast<std::size_t>(0)
	);
}

bool cuda_device_backend::get_device_properties(
	std::size_t id, 
	device_properties &desc
) const
{
	int count;
	cudaGetDeviceCount(&count);

	const auto device = static_cast<int>(id);
	const auto result = device < count;
	if (result)
	{
		cudaDeviceProp prop;
		XMIPP4_CUDA_CHECK( cudaGetDeviceProperties(&prop, device) );

		// Convert
		const auto type = 
			prop.integrated ? device_type::integrated_gpu : device_type::gpu;
		auto location = pci_id_to_string(
			prop.pciBusID, 
			prop.pciDeviceID, 
			prop.pciDomainID
		);

		// Write
		desc.set_name(std::string(prop.name));
		desc.set_physical_location(std::move(location));
		desc.set_type(type);
		desc.set_total_memory_bytes(prop.totalGlobalMem);
	}

	return result;
}

std::shared_ptr<device> 
cuda_device_backend::create_device(std::size_t id)
{
	int count;
	XMIPP4_CUDA_CHECK( cudaGetDeviceCount(&count) );
	if (static_cast<int>(id) >= count)
	{
		throw std::invalid_argument("Invalid device id");
	}

	return std::make_shared<cuda_device>(static_cast<int>(id));
}

bool cuda_device_backend::register_at(device_manager &manager)
{
	return manager.register_backend(std::make_unique<cuda_device_backend>());
}

} // namespace hardware
} // namespace xmipp4
