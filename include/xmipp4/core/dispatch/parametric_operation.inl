// SPDX-License-Identifier: GPL-3.0-only

#include "parametric_operation.hpp"

namespace rex
{

template <typename Derived, typename Traits>
template <typename... Args, typename>
inline
parametric_operation<Derived, Traits>::parametric_operation(Args&&... args)
	: m_shape_policy(std::forward<Args>(args)...)
{
}

template <typename Derived, typename Traits>
inline
const typename parametric_operation<Derived, Traits>::shape_policy_type&
parametric_operation<Derived, Traits>::get_shape_policy() const noexcept
{
	return m_shape_policy;
}

template <typename Derived, typename Traits>
inline
const operation_shape_policy&
parametric_operation<Derived, Traits>
::get_operation_shape_policy() const noexcept
{
	return m_shape_policy;
}

template <typename Derived, typename Traits>
inline
const operation_data_type_policy&
parametric_operation<Derived, Traits>
::get_operation_data_type_policy() const noexcept
{
	return rule_data_type_policy<type_rule>::get();
}

} // namespace rex
