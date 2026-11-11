// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/stream_program.hpp>

#include <rexlib/core/hardware/buffer.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cuda
{

class mock_stream_program final
	: public stream_program
{
public:
	MAKE_CONST_MOCK4(
		launch,
		void(
			span<const std::shared_ptr<rexlib::buffer>>,
			span<const std::shared_ptr<const rexlib::buffer>>,
			span<const std::shared_ptr<rexlib::buffer>>,
			cudaStream_t
		),
		override
	);
};

} // namespace cuda
} // namespace rexlib
