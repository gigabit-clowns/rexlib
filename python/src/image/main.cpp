// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "location.hpp"

namespace xmipp4
{
namespace image
{

namespace py = pybind11;

void bind_image(pybind11::module_ &m)
{
    bind_location(m);
}

} // namespace compute
} // namespace xmipp4
