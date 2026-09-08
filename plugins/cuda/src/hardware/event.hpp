// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/event.hpp>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief CUDA implementation of @ref rexlib::event, backed by a cudaEvent_t.
 *
 * Supports every @ref event_usage_flag_bits value.
 */
class event final
	: public rexlib::event
{
public:
	using handle = cudaEvent_t;

	explicit event(int ordinal);
	event(const event &other) = delete;
	event(event &&other) = delete;
	~event() override;

	event& operator=(const event &other) = delete;
	event& operator=(event &&other) = delete;

	handle get_handle() const noexcept;
	int get_ordinal() const noexcept;

	event_usage_flags get_supported_usage() const noexcept override;

	void wait() const override;
	bool is_signaled() const override;

	/**
	 * @brief Downcast an event to this backend's implementation.
	 *
	 * @param ev The event to be cast.
	 * @return event& The same event, as a CUDA event.
	 *
	 * @throws std::invalid_argument If the event was not created by this
	 * backend.
	 */
	static event& cast(rexlib::event &ev);

	/**
	 * @brief Downcast an event to this backend's implementation.
	 *
	 * @param ev The event to be cast.
	 * @return const event& The same event, as a CUDA event.
	 *
	 * @throws std::invalid_argument If the event was not created by this
	 * backend.
	 */
	static const event& cast(const rexlib::event &ev);

private:
	handle m_event;
	int m_ordinal;
};

} // namespace cuda
} // namespace rexlib
