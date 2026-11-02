// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "preprocessing.hpp"
#include "reconstruction.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_functional(nb::module_ &m)
{
	bind_preprocessing(m);
	bind_reconstruction(m);
}

} // namespace em
} // namespace rexlib
