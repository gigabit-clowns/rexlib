// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "preprocessing.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_functional(nb::module_ &m)
{
	bind_preprocessing(m);
}

} // namespace em
} // namespace rexlib
