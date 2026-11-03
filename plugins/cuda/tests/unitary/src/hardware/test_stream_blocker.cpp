// SPDX-License-Identifier: GPL-3.0-only

#include "test_stream_blocker.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>

namespace rexlib
{
namespace cuda
{

stream_blocker::stream_blocker(cudaStream_t stream)
	: m_is_released(false)
{
	auto until_released = std::make_unique<std::shared_future<void>>(
		m_released.get_future()
	);

	REQUIRE(
		cudaLaunchHostFunc(
			stream,
			&wait_until_released,
			until_released.get()
		) == cudaSuccess
	);

	// The work issued on the stream owns it from here on.
	(void)until_released.release();
}

stream_blocker::~stream_blocker()
{
	release();
}

void stream_blocker::release() noexcept
{
	if (!m_is_released)
	{
		m_is_released = true;
		m_released.set_value();
	}
}

void CUDART_CB stream_blocker::wait_until_released(void *data)
{
	const std::unique_ptr<std::shared_future<void>> until_released(
		static_cast<std::shared_future<void>*>(data)
	);
	until_released->wait();
}

} // namespace cuda
} // namespace rexlib
