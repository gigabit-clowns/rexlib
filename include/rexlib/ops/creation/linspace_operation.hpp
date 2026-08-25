// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/sequence_operation_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

REXLIB_DECLARE_OPERATION_TRAITS(
	linspace,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS(),
	sequence_operation_shape_policy,
	nullary_free_rule<inexact_type_domain>
);

/**
 * @brief Write evenly spaced samples of the segment from start to stop.
 *
 * Unlike @ref arange_operation this is told how many samples it wants and
 * works the spacing out from them, which is what makes the two complementary
 * rather than redundant: one is exact about the step and approximate about
 * the length, the other the other way round.
 *
 * When the stop is included it is written verbatim into the last element
 * rather than being left to fall out of the arithmetic, so that both ends of
 * the segment are exact whatever the spacing rounds to. A single sample has
 * no interval to span and takes the start.
 *
 * The domain is the inexact types. Evenly spaced samples of a segment are
 * generally not whole numbers, so an integer result would be a truncation
 * dressed up as a sequence; @ref arange_operation is the one that enumerates
 * integers. Complex samples are admitted: a segment of the complex plane is
 * as well defined as one of the real line.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API linspace_operation final
	: public parametric_operation<linspace_operation, linspace_operation_traits>
{
public:
	/**
	 * @brief Construct a linspace operation.
	 *
	 * @param start Value of the first element.
	 * @param stop End of the segment being sampled.
	 * @param count Number of samples to write.
	 * @param endpoint Whether @p stop is itself a sample. When it is not,
	 * the samples are those of the half open segment, so that consecutive
	 * blocks of them tile the line without repeating a value.
	 */
	linspace_operation(
		const scalar_value &start,
		const scalar_value &stop,
		std::size_t count,
		bool endpoint = true
	);

	/**
	 * @brief Get the value of the first element.
	 *
	 * @return const scalar_value& The start of the segment.
	 */
	const scalar_value& get_start() const noexcept;

	/**
	 * @brief Get the end of the sampled segment.
	 *
	 * @return const scalar_value& The stop.
	 */
	const scalar_value& get_stop() const noexcept;

	/**
	 * @brief Get the number of samples written.
	 *
	 * @return std::size_t The sample count.
	 */
	std::size_t get_count() const noexcept;

	/**
	 * @brief Check whether the stop is itself a sample.
	 *
	 * @return bool The endpoint flag.
	 */
	bool get_endpoint() const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	scalar_value m_start;
	REXLIB_STD_MEMBER_INTERFACE
	scalar_value m_stop;
	bool m_endpoint;
};
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rexlib
