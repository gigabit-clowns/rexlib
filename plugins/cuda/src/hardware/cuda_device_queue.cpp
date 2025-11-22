// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/hardware/cuda_device_queue.hpp>

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/cuda/hardware/cuda_device.hpp>

#include <utility>

namespace xmipp4
{
namespace hardware
{

cuda_device_queue::cuda_device_queue(cuda_device &device)
{
	XMIPP4_CUDA_CHECK( cudaSetDevice(device.get_index()) );
	XMIPP4_CUDA_CHECK( cudaStreamCreate(&m_stream) );
}

cuda_device_queue::cuda_device_queue(cuda_device_queue &&other) noexcept
	: m_stream(other.m_stream)
{
	other.m_stream = nullptr;
}

cuda_device_queue::~cuda_device_queue()
{
	reset();
}

cuda_device_queue& 
cuda_device_queue::operator=(cuda_device_queue &&other) noexcept
{
	swap(other);
	other.reset();
	return *this;
}

void cuda_device_queue::swap(cuda_device_queue &other) noexcept
{
	std::swap(m_stream, other.m_stream);
}

void cuda_device_queue::reset() noexcept
{
	if (m_stream)
	{
		XMIPP4_CUDA_CHECK( cudaStreamDestroy(m_stream) );
	}
}


cuda_device_queue::handle cuda_device_queue::get_handle() noexcept
{
	return m_stream;
}

void cuda_device_queue::wait_until_completed() const
{
	XMIPP4_CUDA_CHECK( cudaStreamSynchronize(m_stream) );
}

bool cuda_device_queue::is_idle() const noexcept
{
	const auto code = cudaStreamQuery(m_stream);

	bool result;
	switch (code)
	{
	case cudaSuccess:
		result = true;
		break;

	case cudaErrorNotReady:
		result = false;
		break;
	
	default:
		XMIPP4_CUDA_CHECK(code);
		result = false; // To avoid warnings. The line above should throw.
		break;
	}
	return result;
}

} // namespace hardware
} // namespace xmipp4
