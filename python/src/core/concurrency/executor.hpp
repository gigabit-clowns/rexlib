// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/concurrency/thread_pool_executor.hpp>

#include <memory>

namespace rexlib
{

using executor_class = pybind11::class_<executor, std::shared_ptr<executor>>;
using synchronous_executor_class = pybind11::class_<
	synchronous_executor, executor, std::shared_ptr<synchronous_executor>
>;
using thread_pool_executor_class = pybind11::class_<
	thread_pool_executor, executor, std::shared_ptr<thread_pool_executor>
>;

executor_class declare_executor(pybind11::module_ &m);
synchronous_executor_class declare_synchronous_executor(pybind11::module_ &m);
thread_pool_executor_class declare_thread_pool_executor(pybind11::module_ &m);
void define_synchronous_executor(synchronous_executor_class &c);
void define_thread_pool_executor(thread_pool_executor_class &c);

} // namespace rexlib
