// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/random.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/ops/random/fold_in_key_operation.hpp>
#include <rexlib/ops/random/split_key_operation.hpp>

#include <random>

namespace rexlib
{

array make_key(
	std::uint64_t seed,
	const execution_context &context,
	array *out
)
{
	return full(
		make_contiguous_array_descriptor(
			span<const std::size_t>(),
			numerical_type::uint64
		),
		memory_resource_affinity::device,
		seed,
		context,
		out
	);
}

std::uint64_t entropy_seed()
{
	std::random_device device;
	const auto high = static_cast<std::uint32_t>(device());
	const auto low = static_cast<std::uint32_t>(device());
	return (static_cast<std::uint64_t>(high) << 32) | low;
}

array fold_in_key(
	const_array_ref key,
	const_array_ref data,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		ops::fold_in_key_operation(),
		key,
		data,
		context,
		out
	);
}

array split_key(
	const_array_ref key,
	std::size_t count,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::split_key_operation(count),
		key,
		context,
		out
	);
}

} // namespace rexlib
