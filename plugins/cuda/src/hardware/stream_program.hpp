// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/span.hpp>

#include <memory>

#include <cuda_runtime.h>

namespace rexlib
{

class buffer;

namespace cuda
{

/**
 * @brief A program of the CUDA backend: work that is issued on a stream.
 */
class stream_program
	: public rexlib::program
{
public:
	stream_program() noexcept;
	stream_program(const stream_program &other) = delete;
	stream_program(stream_program &&other) = delete;
	~stream_program() override;

	stream_program& operator=(const stream_program &other) = delete;
	stream_program& operator=(stream_program &&other) = delete;

	/**
	 * @brief Issue the work of this program on a stream.
	 *
	 * Returns once the work is issued. It may still be running by then, and
	 * runs after whatever the stream was given before.
	 *
	 * @param output_operands The buffers the program writes.
	 * @param input_operands The buffers the program reads.
	 * @param scratch_operands One buffer for each requirement of
	 * @ref get_scratch_requirements, in that order.
	 * @param stream The stream the work is issued on.
	 *
	 * @throws std::exception If the work could not be issued.
	 */
	virtual void launch(
		span<const std::shared_ptr<rexlib::buffer>> output_operands,
		span<const std::shared_ptr<const rexlib::buffer>> input_operands,
		span<const std::shared_ptr<rexlib::buffer>> scratch_operands,
		cudaStream_t stream
	) const = 0;
};

} // namespace cuda
} // namespace rexlib
