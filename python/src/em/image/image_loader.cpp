// SPDX-License-Identifier: GPL-3.0-only

#include "image_loader.hpp"

#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <memory>

namespace rexlib
{

namespace py = pybind11;

image_loader_class declare_image_loader(pybind11::module_ &m)
{
	return image_loader_class(m, "ImageLoader");
}

executor_image_loader_class
declare_executor_image_loader(pybind11::module_ &m)
{
	return executor_image_loader_class(m, "ExecutorImageLoader");
}

void define_executor_image_loader(executor_image_loader_class &c)
{
	c.def(
		py::init<
			std::shared_ptr<em::image_reader_provider>,
			std::shared_ptr<executor>
		>(),
		py::arg("readers"), py::arg("executor")
	);
}

} // namespace rexlib
