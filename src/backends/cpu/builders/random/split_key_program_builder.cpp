// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/random/split_key_operation.hpp>

#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/kernels/random_key.hpp>
#include <backends/cpu/numerical/load_store.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>

#include <cstdint>

namespace rexlib
{
namespace cpu
{

namespace
{

struct split_key_kernel
{
	void operator()(
		std::uint64_t *result,
		const std::uint64_t *key,
		const multidimensional_index &index
	) const noexcept
	{
		store(result, derive_key(load(key), index[index.get_rank() - 1]));
	}
};

} // anonymous namespace

REXLIB_REGISTER_MULTIDIMENSIONAL_INDEXED_ELEMENTWISE_PROGRAM_BUILDER(
	split_key,
	ops::split_key_operation,
	default_kernel_factory<split_key_kernel>
);

} // namespace cpu
} // namespace rexlib
