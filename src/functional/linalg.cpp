// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/linalg.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/core/layout/index.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <rex/ops/linalg/cross_operation.hpp>
#include <rex/ops/linalg/matmul_operation.hpp>
#include <rex/ops/linalg/matvec_operation.hpp>
#include <rex/ops/linalg/vecdot_operation.hpp>
#include <rex/ops/linalg/vecmat_operation.hpp>

#include <algorithm>
#include <vector>

namespace rexlib
{

namespace
{

/**
 * @brief Resolve the axis a caller gave against an operand.
 *
 * Both operands are broadcast, so the axis is resolved
 * against whichever has more of them: an axis referred from
 * the end means the same thing in the broadcast result.
 */
std::size_t resolve_axis(
	const const_array_ref &x,
	const const_array_ref &y,
	std::ptrdiff_t axis
)
{
	std::vector<std::size_t> extents;
	x.get_descriptor().get_layout().get_extents(extents);
	const auto left_rank = extents.size();
	y.get_descriptor().get_layout().get_extents(extents);
	const auto rank = std::max(left_rank, extents.size());

	return sanitize_index(axis, rank);
}

} // anonymous namespace

array matmul(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::matmul_operation(), x, y, context, out);
}

array matvec(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::matvec_operation(), x, y, context, out);
}

array vecmat(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::vecmat_operation(), x, y, context, out);
}

array vecdot(
	const_array_ref x,
	const_array_ref y,
	std::ptrdiff_t axis,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		ops::vecdot_operation(
			ops::axis_list(1, resolve_axis(x, y, axis))
		),
		x,
		y,
		context,
		out
	);
}

array vecdot(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return vecdot(x, y, -1, context, out);
}

array cross(
	const_array_ref x,
	const_array_ref y,
	std::ptrdiff_t axis,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		ops::cross_operation(resolve_axis(x, y, axis)),
		x,
		y,
		context,
		out
	);
}

array cross(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return cross(x, y, -1, context, out);
}

} // namespace rexlib
