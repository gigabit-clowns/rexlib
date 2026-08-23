// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/caching_allocator/event_recorder.hpp>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief An @ref event_recorder that forwards to one it does not own.
 *
 * An allocator takes ownership of its recorder, which a mock the test needs to
 * keep hold of cannot be given. This is what goes in instead.
 */
class event_recorder_reference final
	: public event_recorder
{
public:
	/**
	 * @brief Construct a reference to a recorder.
	 *
	 * @param target The recorder to forward to. Must outlive this.
	 */
	explicit event_recorder_reference(event_recorder &target) noexcept;
	~event_recorder_reference() override;

	ticket record(const queue_handle &queue) override;
	void release(ticket ticket) noexcept override;
	bool is_complete(ticket ticket) override;
	void wait(ticket ticket) override;
	void enqueue_wait(const queue_handle &queue, ticket ticket) override;

private:
	event_recorder *m_target;
};

} // namespace cuda
} // namespace xmipp4
