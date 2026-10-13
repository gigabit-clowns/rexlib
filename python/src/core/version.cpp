// SPDX-License-Identifier: GPL-3.0-only

#include "version.hpp"

#include <rexlib/core/version.hpp>
#include <rexlib/core/library_version.hpp>

#include <cstdint>
#include <sstream>
#include <tuple>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

namespace rexlib
{

namespace nb = nanobind;

static std::string to_string(const version &v)
{
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

static std::string to_repr(const version &v)
{
	std::ostringstream oss;
	oss << "Version(major=" << v.get_major() << ", "
		<< "minor=" << v.get_minor() << ", "
		<< "patch=" << v.get_patch() << ")";
	return oss.str();
}

static version get_binding_version() noexcept
{
	return version(
		REXLIB_PYTHON_VERSION_MAJOR,
		REXLIB_PYTHON_VERSION_MINOR,
		REXLIB_PYTHON_VERSION_PATCH
	);
}

version_class declare_version(nb::module_ &m)
{
	return version_class(m, "Version");
}

void define_version(version_class &c, nb::module_ &m)
{
	c
		.def(
			nb::init<std::uint32_t, std::uint32_t, std::uint32_t>(),
			nb::arg("major"), nb::arg("minor"), nb::arg("patch")
		)
		.def(nb::self == nb::self)
		.def(nb::self != nb::self)
		.def(nb::self < nb::self)
		.def(nb::self <= nb::self)
		.def(nb::self > nb::self)
		.def(nb::self >= nb::self)
		.def("__str__", &to_string)
		.def("__repr__", &to_repr)
		.def_prop_rw("major", &version::get_major, &version::set_major)
		.def_prop_rw("minor", &version::get_minor, &version::set_minor)
		.def_prop_rw("patch", &version::get_patch, &version::set_patch)
		.def(
			"__getstate__",
			[](const version &v)
			{
				return std::make_tuple(
					v.get_major(),
					v.get_minor(),
					v.get_patch()
				);
			}
		)
		.def(
			"__setstate__",
			[](
				version &v,
				const std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>
					&state
			)
			{
				new (&v) version(
					std::get<0>(state),
					std::get<1>(state),
					std::get<2>(state)
				);
			}
		);

	c.attr("__hash__") = nb::none();

	m.attr("rexlib_version") = nb::cast(get_library_version());
	m.attr("rexlib_binding_version") = nb::cast(get_binding_version());
}

} // namespace rexlib
