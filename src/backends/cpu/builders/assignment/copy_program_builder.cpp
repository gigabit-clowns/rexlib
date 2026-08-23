// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/assignment/copy_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/load_store.hpp>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct copy_kernel
{
	template <typename T, typename Q>
	void operator()(T *result, const Q *x) const noexcept
	{
		cast(result, x);
	}
};

// Convertibility is a property of the static types with no
// numerical_type counterpart, so it stays a backend predicate rather than
// being spelled as a domain. Pivot 0 is the destination and pivot 1 the
// source, as copy_operation's converting rule declares them.
template <typename T, typename Q>
struct copy_support : std::is_convertible<Q, T> {};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER_EX(
	copy,
	ops::copy_operation,
	default_kernel_factory<copy_kernel>,
	rule_type_dispatcher<ops::copy_operation::type_rule, copy_support>
);

} // namespace cpu
} // namespace xmipp4
