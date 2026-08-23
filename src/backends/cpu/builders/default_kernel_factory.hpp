// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/meta/type_list.hpp>

namespace rex
{
namespace cpu
{

/**
 * @brief Kernel factory returning a value-initialized kernel.
 *
 * Suits the stateless kernels of most elementwise operations.
 *
 * @tparam Kernel The per-element functor type to be value-initialized.
 */
template <typename Kernel>
class default_kernel_factory
{
public:
	/**
	 * @brief Create the kernel.
	 *
	 * @param operation The operation the program is built for. Unused.
	 * @param output_types type_list of the output element types. Unused.
	 * @param input_types type_list of the input element types. Unused.
	 * @return Kernel A value-initialized kernel.
	 */
	template <typename Op, typename... Outs, typename... Ins>
	Kernel operator()(
		const Op &operation,
		type_list<Outs...> output_types,
		type_list<Ins...> input_types
	) const;
};

} // namespace cpu
} // namespace rex

#include "default_kernel_factory.inl"
