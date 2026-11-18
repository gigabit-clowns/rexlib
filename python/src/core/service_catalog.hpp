// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/service_catalog.hpp>

#include <memory>

namespace rexlib
{

using service_catalog_class = nanobind::class_<service_catalog>;

service_catalog_class declare_service_catalog(nanobind::module_ &m);
void define_service_catalog(service_catalog_class &c);

} // namespace rexlib
