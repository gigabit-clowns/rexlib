// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/functional/reduction.hpp>

#include <xmipp4/core/dispatch/execute.hpp>
#include <xmipp4/core/layout/index.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/ndarray/array_descriptor.hpp>
#include <xmipp4/ops/policies/reduction_operation_shape_policy.hpp>
#include <xmipp4/ops/reduction/all_operation.hpp>
#include <xmipp4/ops/reduction/amax_operation.hpp>
#include <xmipp4/ops/reduction/amin_operation.hpp>
#include <xmipp4/ops/reduction/any_operation.hpp>
#include <xmipp4/ops/reduction/argmax_operation.hpp>
#include <xmipp4/ops/reduction/argmin_operation.hpp>
#include <xmipp4/ops/reduction/count_nonzero_operation.hpp>
#include <xmipp4/ops/reduction/mean_operation.hpp>
#include <xmipp4/ops/reduction/product_operation.hpp>
#include <xmipp4/ops/reduction/sum_operation.hpp>

#include <numeric>
#include <vector>

namespace rex
{

namespace
{

std::size_t get_rank(const const_array_ref &x)
{
	std::vector<std::size_t> extents;
	x.get_descriptor().get_layout().get_extents(extents);
	return extents.size();
}

/**
 * @brief Turn the axes a caller gave into the ones an operation holds.
 *
 * Negative axes are referred from the end. An operation is a rigid
 * description of the work to be done, so this resolution happens here,
 * where the array and therefore its rank are at hand, rather than being
 * carried into the operation to be interpreted later.
 */
ops::reduction_operation_shape_policy::axis_list_type resolve_axes(
	const const_array_ref &x,
	span<const std::ptrdiff_t> axes
)
{
	const auto rank = get_rank(x);

	ops::reduction_operation_shape_policy::axis_list_type result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(sanitize_index(axis, rank));
	}

	return result;
}

/**
 * @brief Name every axis of an array.
 *
 * Reducing over all of them is spelled out here rather than left as a mode
 * of the operation, because it only means something once a rank is known.
 */
ops::reduction_operation_shape_policy::axis_list_type every_axis(
	const const_array_ref &x
)
{
	ops::reduction_operation_shape_policy::axis_list_type result(get_rank(x));
	std::iota(result.begin(), result.end(), std::size_t(0));
	return result;
}

} // anonymous namespace

array sum(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::sum_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array sum(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::sum_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array product(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::product_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array product(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::product_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array amin(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::amin_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array amin(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::amin_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array amax(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::amax_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array amax(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::amax_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array all(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::all_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array all(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::all_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array any(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::any_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array any(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::any_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array argmin(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::argmin_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array argmin(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::argmin_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array argmax(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::argmax_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array argmax(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::argmax_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array count_nonzero(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::count_nonzero_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array count_nonzero(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::count_nonzero_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array mean(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	bool keep_dimensions,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::mean_operation(resolve_axes(x, axes), keep_dimensions),
		x,
		context,
		out
	);
}

array mean(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::mean_operation(every_axis(x)),
		x,
		context,
		out
	);
}

} // namespace rex
