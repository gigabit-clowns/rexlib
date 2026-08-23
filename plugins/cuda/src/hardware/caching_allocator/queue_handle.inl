// SPDX-License-Identifier: GPL-3.0-only

#include "queue_handle.hpp"

#include "../command_queue.hpp"

namespace xmipp4
{
namespace cuda
{

inline
queue_handle::queue_handle() noexcept
	: m_stream(nullptr)
	, m_ordinal(-1)
{
}

inline
queue_handle::queue_handle(cudaStream_t stream, int ordinal) noexcept
	: m_stream(stream)
	, m_ordinal(ordinal)
{
}

inline
queue_handle::queue_handle(const command_queue &queue) noexcept
	: m_stream(queue.get_handle())
	, m_ordinal(queue.get_ordinal())
{
}

inline
cudaStream_t queue_handle::get_stream() const noexcept
{
	return m_stream;
}

inline
int queue_handle::get_ordinal() const noexcept
{
	return m_ordinal;
}

inline
queue_handle::operator bool() const noexcept
{
	return m_stream != nullptr;
}

} // namespace cuda
} // namespace xmipp4
