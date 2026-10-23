// SPDX-License-Identifier: GPL-3.0-only

#include "parametric_operation.hpp"

namespace rexlib
{

template <typename Derived, typename Traits>
template <typename... Args, typename>
inline
parametric_operation<Derived, Traits>::parametric_operation(Args&&... args)
	: m_contract(std::forward<Args>(args)...)
{
	m_contract.check_parameters(
		basic_operation<Derived, Traits>::get_static_descriptor()
	);
}

template <typename Derived, typename Traits>
inline
const typename parametric_operation<Derived, Traits>::parameters_type&
parametric_operation<Derived, Traits>::get_parameters() const noexcept
{
	return m_contract.get_parameters();
}

template <typename Derived, typename Traits>
inline
const operation_contract&
parametric_operation<Derived, Traits>::get_contract() const noexcept
{
	return m_contract;
}

} // namespace rexlib
