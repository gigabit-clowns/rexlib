// SPDX-License-Identifier: GPL-3.0-only

#include "trivial_operation.hpp"

namespace rexlib
{

template <typename Derived, typename Traits>
inline
const operand_contract&
trivial_operation<Derived, Traits>::get_contract() const noexcept
{
	return shared_rule_contract<shape_rule, type_rule>::get();
}

} // namespace rexlib
