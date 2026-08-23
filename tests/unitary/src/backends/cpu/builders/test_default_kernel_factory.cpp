// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/default_kernel_factory.hpp>

#include <xmipp4/core/meta/type_list.hpp>

using namespace rex;
using namespace rex::cpu;

namespace
{

// A kernel with an observable default-constructed state.
struct probe_kernel
{
	int value = 7;
};

// A stand-in operation type; default_kernel_factory ignores it.
struct dummy_operation {};

} // namespace

TEST_CASE(
	"default_kernel_factory returns a value-initialized kernel",
	"[default_kernel_factory]"
)
{
	const default_kernel_factory<probe_kernel> factory;
	const dummy_operation operation;

	const auto kernel = factory(
		operation,
		type_list<float>(),
		type_list<float, float>()
	);

	CHECK( kernel.value == probe_kernel().value );
}
