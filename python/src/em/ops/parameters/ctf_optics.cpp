// SPDX-License-Identifier: GPL-3.0-only

#include "ctf_optics.hpp"

#include <nanobind/stl/string.h>

#include <sstream>
#include <string>

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

static std::string to_repr(const ctf_optics &optics)
{
	std::ostringstream oss;
	oss << "CtfOptics(voltage=" << optics.get_voltage()
		<< ", spherical_aberration=" << optics.get_spherical_aberration()
		<< ", amplitude_contrast=" << optics.get_amplitude_contrast() << ")";
	return oss.str();
}

ctf_optics_class declare_ctf_optics(nb::module_ &m)
{
	return ctf_optics_class(m, "CtfOptics");
}

void define_ctf_optics(ctf_optics_class &c)
{
	c
		.def(
			nb::init<double, double, double>(),
			nb::arg("voltage"),
			nb::arg("spherical_aberration"),
			nb::arg("amplitude_contrast")
		)
		.def_prop_ro("voltage", &ctf_optics::get_voltage)
		.def_prop_ro(
			"spherical_aberration",
			&ctf_optics::get_spherical_aberration
		)
		.def_prop_ro("amplitude_contrast", &ctf_optics::get_amplitude_contrast)
		.def_prop_ro("wavelength", &ctf_optics::get_wavelength)
		.def("__repr__", &to_repr);
}

} // namespace em
} // namespace rexlib
