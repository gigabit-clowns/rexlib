// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "image/main.hpp"

namespace rexlib
{

void bind_em(pybind11::module_ &m)
{
	auto image_module = m.def_submodule("image");
	bind_image(image_module);
}

} // namespace rexlib
