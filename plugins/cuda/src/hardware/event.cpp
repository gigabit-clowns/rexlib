// SPDX-License-Identifier: GPL-3.0-only

#include "event.hpp"

#include "device_guard.hpp"
#include "error.hpp"

#include <stdexcept>

namespace rexlib
{
namespace cuda
{

event::event(int ordinal)
	: m_event(nullptr)
	, m_ordinal(ordinal)
{
	// Timing is never read from these events, and a blocking host wait
	// yields the CPU instead of spinning.
	const device_guard guard(ordinal);
	REXLIB_CUDA_CHECK(
		cudaEventCreateWithFlags(
			&m_event,
			cudaEventDisableTiming | cudaEventBlockingSync
		)
	);
}

event::~event()
{
	// Acts on the device that owns the event, so the current one is
	// irrelevant here.
	REXLIB_CUDA_CHECK_NO_THROW( cudaEventDestroy(m_event) );
}

event::handle event::get_handle() const noexcept
{
	return m_event;
}

int event::get_ordinal() const noexcept
{
	return m_ordinal;
}

event_usage_flags event::get_supported_usage() const noexcept
{
	return {
		event_usage_flag_bits::host_query,
		event_usage_flag_bits::host_wait,
		event_usage_flag_bits::device_wait,
		event_usage_flag_bits::cross_device_wait
	};
}

void event::wait() const
{
	REXLIB_CUDA_CHECK( cudaEventSynchronize(m_event) );
}

bool event::is_signaled() const
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
		REXLIB_CUDA_CHECK(code);
		result = false; // To avoid warnings. The above line should throw.
		break;
	}
	return result;
}

event& event::cast(rexlib::event &ev)
{
	auto *result = dynamic_cast<event*>(&ev);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided event was not created by the CUDA backend."
		);
	}

	return *result;
}

const event& event::cast(const rexlib::event &ev)
{
	const auto *result = dynamic_cast<const event*>(&ev);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided event was not created by the CUDA backend."
		);
	}

	return *result;
}

} // namespace cuda
} // namespace rexlib
