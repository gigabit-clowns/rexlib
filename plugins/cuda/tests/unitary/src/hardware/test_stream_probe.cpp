// SPDX-License-Identifier: GPL-3.0-only

#include "test_stream_probe.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

namespace rexlib
{
namespace cuda
{

stream_probe::stream_probe()
	: m_finished_count(std::make_shared<std::atomic<int>>(0))
	, m_issued_count(0)
{
}

stream_probe::~stream_probe() = default;

void stream_probe::issue(cudaStream_t stream)
{
	auto counter = std::make_unique<std::shared_ptr<std::atomic<int>>>(
		m_finished_count
	);

	REQUIRE(
		cudaLaunchHostFunc(stream, &finish, counter.get()) == cudaSuccess
	);

	// The work issued on the stream owns it from here on.
	(void)counter.release();
	++m_issued_count;
}

bool stream_probe::has_finished() const noexcept
{
	return m_issued_count > 0 && m_finished_count->load() == m_issued_count;
}

void CUDART_CB stream_probe::finish(void *data)
{
	const std::unique_ptr<std::shared_ptr<std::atomic<int>>> counter(
		static_cast<std::shared_ptr<std::atomic<int>>*>(data)
	);

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	++(**counter);
}

} // namespace cuda
} // namespace rexlib
