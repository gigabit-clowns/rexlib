// SPDX-License-Identifier: GPL-3.0-only

#include "operand_data_types.hpp"

namespace rex
{
namespace cpu
{

template <std::size_t Count>
void extract_data_types(
	std::array<numerical_type, Count> &types,
	span<const operand_signature> signatures
) noexcept
{
	for (std::size_t i = 0; i < Count; ++i)
	{
		types[i] = signatures[i].get_data_type();
	}
}

} // namespace cpu
} // namespace rex
