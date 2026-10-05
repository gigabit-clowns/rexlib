// SPDX-License-Identifier: GPL-3.0-only

#include "image_saver.hpp"

#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/em/image/image_writer_provider.hpp>

#include <memory>

namespace rexlib
{

namespace py = pybind11;

image_saver_class declare_image_saver(pybind11::module_ &m)
{
	return image_saver_class(m, "ImageSaver");
}

executor_image_saver_class declare_executor_image_saver(pybind11::module_ &m)
{
	return executor_image_saver_class(m, "ExecutorImageSaver");
}

void define_executor_image_saver(executor_image_saver_class &c)
{
	c.def(
		py::init<
			std::shared_ptr<em::image_writer_provider>,
			std::shared_ptr<executor>
		>(),
		py::arg("writers"), py::arg("executor")
	);
}

} // namespace rexlib
