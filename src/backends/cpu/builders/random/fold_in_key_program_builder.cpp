// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/random/fold_in_key_operation.hpp>

#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/kernels/random_key.hpp>
#include <backends/cpu/numerical/load_store.hpp>

#include <cstdint>

namespace rexlib
{
namespace cpu
{

namespace
{

struct fold_in_key_kernel
{
	void operator()(
		std::uint64_t *result,
		const std::uint64_t *key,
		const std::uint64_t *data
	) const noexcept
	{
		store(result, derive_key(load(key), load(data)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	fold_in_key,
	ops::fold_in_key_operation,
	default_kernel_factory<fold_in_key_kernel>
);

} // namespace cpu
} // namespace rexlib
