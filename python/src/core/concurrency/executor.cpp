// SPDX-License-Identifier: GPL-3.0-only

#include "executor.hpp"

#include <cstddef>

namespace rexlib
{

namespace py = pybind11;

executor_class declare_executor(pybind11::module_ &m)
{
	return executor_class(m, "Executor");
}

synchronous_executor_class declare_synchronous_executor(pybind11::module_ &m)
{
	return synchronous_executor_class(m, "SynchronousExecutor");
}

thread_pool_executor_class declare_thread_pool_executor(pybind11::module_ &m)
{
	return thread_pool_executor_class(m, "ThreadPoolExecutor");
}

void define_synchronous_executor(synchronous_executor_class &c)
{
	c.def(py::init<>());
}

void define_thread_pool_executor(thread_pool_executor_class &c)
{
	c.def(py::init<std::size_t>(), py::arg("worker_count"));
}

} // namespace rexlib
