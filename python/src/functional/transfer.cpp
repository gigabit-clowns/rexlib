// SPDX-License-Identifier: GPL-3.0-only

#include "transfer.hpp"

#include "../core/ndarray/array_ref_caster.hpp"

#include <rexlib/functional/transfer.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

namespace rexlib
{

namespace nb = nanobind;

void bind_transfer(nb::module_ &m)
{
	m.def(
		"transfer", &rexlib::transfer,
		nb::arg("input"), nb::arg("affinity"), nb::arg("context")
	);
	m.def(
		"transfer_copy", &rexlib::transfer_copy,
		nb::arg("input"), nb::arg("affinity"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"to_device", &rexlib::to_device,
		nb::arg("input"), nb::arg("context")
	);
	m.def(
		"to_device_copy", &rexlib::to_device_copy,
		nb::arg("input"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"to_host", &rexlib::to_host,
		nb::arg("input"), nb::arg("context")
	);
	m.def(
		"to_host_copy", &rexlib::to_host_copy,
		nb::arg("input"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
}

} // namespace rexlib
