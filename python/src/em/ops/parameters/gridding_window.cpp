// SPDX-License-Identifier: GPL-3.0-only

#include "gridding_window.hpp"

#include <nanobind/stl/string.h>

#include <sstream>
#include <string>

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

static std::string to_repr(const gridding_window &window)
{
	std::ostringstream oss;
	oss << "GriddingWindow(profile=" << window.get_profile()
		<< ", half_width=" << window.get_half_width()
		<< ", shape=" << window.get_shape() << ")";
	return oss.str();
}

gridding_window_class declare_gridding_window(nb::module_ &m)
{
	return gridding_window_class(m, "GriddingWindow");
}

void define_gridding_window(gridding_window_class &c)
{
	c
		.def_static("nearest", &gridding_window::nearest)
		.def_static("linear", &gridding_window::linear)
		.def_static(
			"kaiser_bessel", &gridding_window::kaiser_bessel,
			nb::arg("half_width"), nb::arg("shape")
		)
		.def_prop_ro("profile", &gridding_window::get_profile)
		.def_prop_ro("half_width", &gridding_window::get_half_width)
		.def_prop_ro("shape", &gridding_window::get_shape)
		.def("__repr__", &to_repr);
}

} // namespace em
} // namespace rexlib
