// SPDX-License-Identifier: GPL-3.0-only

#include "creation.hpp"

#include "../core/ndarray/array_ref_caster.hpp"
#include "../core/numerical/float16_caster.hpp"

#include <rexlib/functional/creation.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

#include <nanobind/stl/complex.h>
#include <nanobind/stl/string.h>

#include <string>

namespace rexlib
{

namespace nb = nanobind;

// Kept out of the dispatch below, which would otherwise build this message
// once per numerical type. Nothing here depends on the type being cast to.
[[noreturn]]
static void throw_scalar_cast_error(numerical_type type, const nb::object &value)
{
	const auto given =
		nb::cast<std::string>(value.type().attr("__name__"));
	auto message =
		"cannot use a '" + given + "' as a " + to_string(type) + " value";
	if (type == numerical_type::char8)
	{
		message +=
			"; char8 holds characters, so it takes a "
			"single-character str or bytes";
	}
	throw nb::type_error(message.c_str());
}

template <typename T>
static bool load_scalar(const nb::object &value, T &result)
{
	return nb::try_cast(value, result);
}

// Takes, besides True and False, any number with a truth value.
template <>
bool load_scalar<bool>(const nb::object &value, bool &result)
{
	if (nb::try_cast(value, result))
	{
		return true;
	}

	auto *type = reinterpret_cast<PyTypeObject*>(value.type().ptr());
	if (value.is_none() || PyType_GetSlot(type, Py_nb_bool) == nullptr)
	{
		return false;
	}

	const auto truth = PyObject_IsTrue(value.ptr());
	if (truth < 0)
	{
		PyErr_Clear();
		return false;
	}

	result = truth != 0;
	return true;
}

static scalar_value make_scalar_value(numerical_type type, const nb::object &value)
{
	return dispatch_numerical_types(
		[&value, type](auto tag)
		{
			using T = typename decltype(tag)::type;
			T result;
			if (!load_scalar(value, result))
			{
				throw_scalar_cast_error(type, value);
			}
			return scalar_value(result);
		},
		type
	);
}

static array py_full(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const nb::object &fill_value,
	const execution_context &context,
	array *out
)
{
	const auto scalar = make_scalar_value(descriptor.get_data_type(), fill_value);
	return rexlib::full(std::move(descriptor), affinity, scalar, context, out);
}

static void py_fill(array &out, const nb::object &fill_value, const execution_context &context)
{
	const auto scalar = make_scalar_value(out.get_descriptor().get_data_type(), fill_value);
	rexlib::fill(out, scalar, context);
}

void bind_creation(nb::module_ &m)
{
	m.def(
		"empty", &rexlib::empty,
		nb::arg("descriptor"), nb::arg("affinity"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"zeros", &rexlib::zeros,
		nb::arg("descriptor"), nb::arg("affinity"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"ones", &rexlib::ones,
		nb::arg("descriptor"), nb::arg("affinity"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"full", &py_full,
		nb::arg("descriptor"), nb::arg("affinity"), nb::arg("fill_value"),
		nb::arg("context"), nb::arg("out").none() = nb::none()
	);
	m.def(
		"copy", &rexlib::copy,
		nb::arg("source"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"fill", &py_fill,
		nb::arg("out"), nb::arg("fill_value"), nb::arg("context")
	);
}

} // namespace rexlib
