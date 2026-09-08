// SPDX-License-Identifier: GPL-3.0-only

#include "command_queue.hpp"

#include "device_guard.hpp"
#include "error.hpp"
#include "event.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/hardware/command.hpp>

#include <stdexcept>

namespace rexlib
{
namespace cuda
{

command_queue::command_queue(int ordinal)
	: m_stream(nullptr)
	, m_ordinal(ordinal)
{
	// Non-blocking, so that queues are only ever ordered against each other
	// through events and never implicitly through the legacy default stream.
	const device_guard guard(ordinal);
	REXLIB_CUDA_CHECK(
		cudaStreamCreateWithFlags(&m_stream, cudaStreamNonBlocking)
	);
}

command_queue::~command_queue()
{
	// Acts on the device that owns the stream, so the current one is
	// irrelevant here.
	REXLIB_CUDA_CHECK_NO_THROW( cudaStreamDestroy(m_stream) );
}

command_queue::handle command_queue::get_handle() const noexcept
{
	return m_stream;
}

int command_queue::get_ordinal() const noexcept
{
	return m_ordinal;
}

void command_queue::synchronize() const
{
	REXLIB_CUDA_CHECK( cudaStreamSynchronize(m_stream) );
}

void command_queue::submit(const command &cmd)
{
	if (!cmd.get_program())
	{
		throw std::invalid_argument(
			"command_queue::submit: Provided command does not have an "
			"associated program."
		);
	}

	throw invalid_operation_error(
		"The CUDA backend does not implement any program yet."
	);
}

void command_queue::signal(rexlib::event &ev)
{
	auto &cuda_event = event::cast(ev);
	REXLIB_CUDA_CHECK(
		cudaEventRecord(cuda_event.get_handle(), m_stream)
	);
}

void command_queue::wait(const rexlib::event &ev)
{
	const auto &cuda_event = event::cast(ev);
	REXLIB_CUDA_CHECK(
		cudaStreamWaitEvent(
			m_stream,
			cuda_event.get_handle(),
			cudaEventWaitDefault
		)
	);
}

command_queue& command_queue::cast(rexlib::command_queue &queue)
{
	auto *result = dynamic_cast<command_queue*>(&queue);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided command queue was not created by the CUDA backend."
		);
	}

	return *result;
}

const command_queue& command_queue::cast(const rexlib::command_queue &queue)
{
	const auto *result = dynamic_cast<const command_queue*>(&queue);
	if (!result)
	{
		throw std::invalid_argument(
			"The provided command queue was not created by the CUDA backend."
		);
	}

	return *result;
}

} // namespace cuda
} // namespace rexlib
