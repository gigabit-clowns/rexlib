// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

/**
 * @brief The type a reduction accumulator over storage type T holds.
 *
 * The widening rule of @ref element_compute_type under the name a reduction
 * reads it by: an accumulator is typed on the computation type rather than
 * on the storage type, which is what keeps a reduction of half precision
 * data from accumulating rounding error at every step.
 *
 * @tparam T The storage element type.
 */
template <typename T>
struct reduction_compute_type
	: element_compute_type<T>
{
};

} // namespace cpu
} // namespace rex
