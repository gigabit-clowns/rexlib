// SPDX-License-Identifier: GPL-3.0-only

#include "test_queue.hpp"

#include <cstdint>

namespace xmipp4
{
namespace cuda
{

queue_handle make_test_queue(unsigned index, int ordinal) noexcept
{
	// Far enough from zero that a handle standing for a queue can never be
	// mistaken for the one standing for no queue.
	const auto value = static_cast<std::uintptr_t>(index + 1) << 12;
	return queue_handle(reinterpret_cast<cudaStream_t>(value), ordinal);
}

} // namespace cuda
} // namespace xmipp4
