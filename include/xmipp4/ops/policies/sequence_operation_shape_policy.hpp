// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/operation_shape_policy.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace xmipp4
{
namespace ops
{

/**
 * @brief Shape policy for operations writing a one dimensional sequence.
 *
 * An operation of this family takes no input: what it writes is decided
 * entirely by the numbers it was built with, so its length cannot be read
 * off an operand and is a parameter instead. The caller that asked for a
 * range is the one that can work the length out from it, which is why the
 * functional layer spells it out before an operation is ever built. An
 * operation is a rigid description of work, not a request to be interpreted
 * later.
 *
 * The rank is one because that is the shape the family produces: a sequence
 * is enumerated along a single axis, and anything else is a reshape of it.
 */
class REXLIB_API sequence_operation_shape_policy final
	: public operation_shape_policy
{
public:
	/**
	 * @brief Construct a policy producing a sequence of a given length.
	 *
	 * @param count Number of elements in the sequence. Zero is admissible
	 * and describes an empty array.
	 */
	explicit sequence_operation_shape_policy(std::size_t count) noexcept;

	/**
	 * @brief Get the length of the sequence.
	 *
	 * @return std::size_t The number of elements.
	 */
	std::size_t get_count() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	std::size_t m_count;
};

} // namespace ops
} // namespace xmipp4
