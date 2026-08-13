// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/service_catalog.hpp>

#include <memory>

namespace xmipp4
{

using service_catalog_class = pybind11::class_<service_catalog>;

service_catalog_class declare_service_catalog(pybind11::module_ &m);
void define_service_catalog(service_catalog_class &c);

} // namespace xmipp4
