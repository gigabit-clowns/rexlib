// SPDX-License-Identifier: GPL-3.0-only

#include "command_queue.hpp"

#include "caching_allocator/buffer.hpp"
#include "device_guard.hpp"
#include "error.hpp"
#include "event_recorder.hpp"
#include "stream_program.hpp"
#include "stream_timeline.hpp"

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <memory>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

namespace
{

/// Check whether every buffer was allocated by this backend. Written for
/// both the buffers a command writes and the ones it only reads.
template <typename Buffer>
bool are_own_buffers(span<const std::shared_ptr<Buffer>> operands) noexcept
{
	for (const auto &operand : operands)
	{
		if (dynamic_cast<const buffer*>(operand.get()) == nullptr)
		{
			return false;
		}
	}

	return true;
}

/// Tell buffers of this backend that a command uses them.
template <typename Buffer>
void record_use(
	span<const std::shared_ptr<Buffer>> operands,
	const command_token &token
)
{
	for (const auto &operand : operands)
	{
		static_cast<const buffer&>(*operand).record_use(token);
	}
}

} // anonymous namespace

command_queue::command_queue(
	int ordinal,
	std::shared_ptr<event_recorder> recorder
)
	: m_timeline(std::make_shared<stream_timeline>(std::move(recorder)))
	, m_stream(nullptr)
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

const std::shared_ptr<stream_timeline>&
command_queue::get_timeline() const noexcept
{
	return m_timeline;
}

void command_queue::synchronize() const
{
	REXLIB_CUDA_CHECK( cudaStreamSynchronize(m_stream) );
}

command_token command_queue::submit(command cmd)
{
	const auto &program = cmd.get_program();
	if (!program)
	{
		throw std::invalid_argument(
			"command_queue::submit: Provided command does not have an "
			"associated program."
		);
	}

	const auto *launchable = dynamic_cast<const stream_program*>(program.get());
	if (!launchable)
	{
		throw std::invalid_argument(
			"command_queue::submit: The program of the command was not "
			"created by the CUDA backend."
		);
	}

	for (const auto &dependency : cmd.get_dependencies())
	{
		wait_for(dependency);
	}

	try
	{
		launchable->launch(
			cmd.get_outputs(),
			cmd.get_inputs(),
			cmd.get_scratch(),
			m_stream
		);

		return track(cmd);
	}
	catch (...)
	{
		// Work may have been issued that nothing keeps track of, so it is
		// finished before anyone can drop the memory it runs on.
		REXLIB_CUDA_CHECK_NO_THROW( cudaStreamSynchronize(m_stream) );
		throw;
	}
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

void command_queue::wait_for(const command_token &dependency) const
{
	const auto &timeline = dependency.get_timeline();

	// The stream runs in order, so a command of this queue is waited for by
	// issuing the work after it.
	if (!timeline || timeline == m_timeline)
	{
		return;
	}

	const auto *other = dynamic_cast<const stream_timeline*>(timeline.get());
	if (other)
	{
		other->enqueue_wait(m_stream, dependency.get_id());
	}
	else
	{
		dependency.wait();
	}
}

command_token command_queue::track(const command &cmd) const
{
	// Nobody would keep the memory of a buffer of another kind until the
	// work ends, so the work ends here.
	if (!are_own_buffers(cmd.get_outputs()) ||
	    !are_own_buffers(cmd.get_inputs()) ||
	    !are_own_buffers(cmd.get_scratch()))
	{
		synchronize();
		return command_token();
	}

	const command_token result(m_timeline, m_timeline->record(m_stream));
	record_use(cmd.get_outputs(), result);
	record_use(cmd.get_inputs(), result);
	record_use(cmd.get_scratch(), result);

	return result;
}

} // namespace cuda
} // namespace rexlib
