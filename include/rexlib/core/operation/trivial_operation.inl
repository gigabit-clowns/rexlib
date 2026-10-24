// SPDX-License-Identifier: GPL-3.0-only

#include "trivial_operation.hpp"

namespace rexlib
{

template <typename Derived, typename Traits>
inline
const operation_signature&
trivial_operation<Derived, Traits>::get_signature() const noexcept
{
	return shared_rule_signature<shape_rule, type_rule>::get();
}

} // namespace rexlib
