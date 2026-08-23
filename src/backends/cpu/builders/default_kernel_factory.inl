// SPDX-License-Identifier: GPL-3.0-only

#include "default_kernel_factory.hpp"

namespace rex
{
namespace cpu
{

template <typename Kernel>
template <typename Op, typename... Outs, typename... Ins>
Kernel default_kernel_factory<Kernel>::operator()(
	const Op& /*operation*/,
	type_list<Outs...> /*output_types*/,
	type_list<Ins...> /*input_types*/
) const
{
	return Kernel();
}

} // namespace cpu
} // namespace rex
