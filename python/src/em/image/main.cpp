// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "image_location.hpp"
#include "image_read.hpp"
#include "image_read_format_manager.hpp"
#include "image_write.hpp"
#include "image_write_format_manager.hpp"

namespace rexlib
{

void bind_image(pybind11::module_ &m)
{
	auto image_location = declare_image_location(m);
	declare_image_read_format_manager(m);
	declare_image_write_format_manager(m);

	define_image_location(image_location);
	define_image_read_format_manager(m);
	define_image_write_format_manager(m);

	bind_image_read(m);
	bind_image_write(m);
}

} // namespace rexlib
