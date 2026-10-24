// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/reduction.hpp>

#include "array_axes.hpp"

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>
#include <rexlib/ops/reduction/all_operation.hpp>
#include <rexlib/ops/reduction/amax_operation.hpp>
#include <rexlib/ops/reduction/amin_operation.hpp>
#include <rexlib/ops/reduction/any_operation.hpp>
#include <rexlib/ops/reduction/argmax_operation.hpp>
#include <rexlib/ops/reduction/argmin_operation.hpp>
#include <rexlib/ops/reduction/count_nonzero_operation.hpp>
#include <rexlib/ops/reduction/mean_operation.hpp>
#include <rexlib/ops/reduction/product_operation.hpp>
#include <rexlib/ops/reduction/sum_operation.hpp>

namespace rexlib
{

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

} // namespace rexlib
