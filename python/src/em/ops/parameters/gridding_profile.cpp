// SPDX-License-Identifier: GPL-3.0-only

#include "gridding_profile.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

gridding_profile_class declare_gridding_profile(nb::module_ &m)
{
	return gridding_profile_class(m, "GriddingProfile");
}

void define_gridding_profile(gridding_profile_class &c)
{
	c
		.value("box", gridding_profile::box)
		.value("triangle", gridding_profile::triangle)
		.value("kaiser_bessel", gridding_profile::kaiser_bessel);
}

} // namespace em
} // namespace rexlib
