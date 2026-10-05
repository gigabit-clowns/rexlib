// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/layout/index_table.hpp>

namespace rexlib
{

using index_table_class = pybind11::class_<index_table>;

index_table_class declare_index_table(pybind11::module_ &m);
void define_index_table(index_table_class &c);

} // namespace rexlib
