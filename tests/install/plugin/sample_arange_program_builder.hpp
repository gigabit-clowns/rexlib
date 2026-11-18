// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/program_builder.hpp>

#include <memory>

namespace sample
{

/**
 * @brief Builds the sample backend's program for arange into int32 on host
 * accessible memory.
 */
class sample_arange_program_builder final
	: public rexlib::program_builder
{
public:
	rexlib::operation_id get_operation_id() const noexcept override;

	rexlib::backend_priority get_suitability(
		const rexlib::operation &operation,
		rexlib::span<const rexlib::operand_descriptor> output_descriptors,
		rexlib::span<const rexlib::operand_descriptor> input_descriptors,
		rexlib::command_queue &queue
	) const override;

	std::shared_ptr<rexlib::program> build(
		const rexlib::operation &operation,
		rexlib::span<const rexlib::operand_descriptor> output_descriptors,
		rexlib::span<const rexlib::operand_descriptor> input_descriptors,
		rexlib::command_queue &queue,
		rexlib::program_cache *cache
	) const override;
};

} // namespace sample
