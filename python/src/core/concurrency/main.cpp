// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "completion.hpp"
#include "executor.hpp"

namespace rexlib
{

void bind_concurrency(pybind11::module_ &m)
{
	auto completion = declare_completion(m);
	declare_executor(m);
	auto synchronous_executor = declare_synchronous_executor(m);
	auto thread_pool_executor = declare_thread_pool_executor(m);

	define_completion(completion);
	define_synchronous_executor(synchronous_executor);
	define_thread_pool_executor(thread_pool_executor);
}

} // namespace rexlib
