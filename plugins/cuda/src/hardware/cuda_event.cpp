// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/hardware/cuda_event.hpp>

#include <xmipp4/cuda/hardware/cuda_error.hpp>
#include <xmipp4/cuda/hardware/cuda_device_queue.hpp>

#include <utility>

namespace xmipp4
{
namespace hardware
{

cuda_event::cuda_event()
{
	XMIPP4_CUDA_CHECK( cudaEventCreate(&m_event) );
}

cuda_event::cuda_event(cuda_event &&other) noexcept
	: m_event(other.m_event)
{
	other.m_event = nullptr;
}

cuda_event::~cuda_event()
{
	reset();
}

cuda_event& 
cuda_event::operator=(cuda_event &&other) noexcept
{
	swap(other);
	other.reset();
	return *this;
}

void cuda_event::swap(cuda_event &other) noexcept
{
	std::swap(m_event, other.m_event);
}

void cuda_event::reset() noexcept
{
	if (m_event)
	{
		XMIPP4_CUDA_CHECK( cudaEventDestroy(m_event) );
	}
}

cuda_event::handle cuda_event::get_handle() noexcept
{
	return m_event;
}



void cuda_event::signal(device_queue &queue)
{
	signal(dynamic_cast<cuda_device_queue&>(queue));
}

void cuda_event::signal(cuda_device_queue &queue)
{
	XMIPP4_CUDA_CHECK( cudaEventRecord(m_event, queue.get_handle()) );
}

void cuda_event::wait() const
{
	XMIPP4_CUDA_CHECK( cudaEventSynchronize(m_event) );
}

void cuda_event::wait(device_queue &queue) const
{
	wait(dynamic_cast<cuda_device_queue&>(queue));
}

void cuda_event::wait(cuda_device_queue &queue) const
{
	XMIPP4_CUDA_CHECK(
		cudaStreamWaitEvent(queue.get_handle(), m_event, cudaEventWaitDefault)
	);
}

bool cuda_event::is_signaled() const
{
	const auto code = cudaEventQuery(m_event);

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
		result = false; // To avoid warnings. The above line should throw.
		break;
	}
	return result;
}

} // namespace hardware
} // namespace xmipp4
