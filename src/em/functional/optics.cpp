// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/functional/optics.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/em/ops/optics/ctf_1d_operation.hpp>
#include <rexlib/em/ops/optics/ctf_2d_operation.hpp>

namespace rexlib
{
namespace em
{

array ctf_1d(
	const_array_ref parameters,
	std::size_t extent,
	const ctf_optics &optics,
	double pixel_size,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ctf_1d_operation(extent, optics, pixel_size),
		parameters,
		context,
		out
	);
}

array ctf_2d(
	const_array_ref parameters,
	std::size_t extent,
	const ctf_optics &optics,
	double pixel_size,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ctf_2d_operation(extent, optics, pixel_size),
		parameters,
		context,
		out
	);
}

} // namespace em
} // namespace rexlib
