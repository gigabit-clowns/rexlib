// SPDX-License-Identifier: GPL-3.0-only

#include "parametric_operation.hpp"

namespace rexlib
{

template <typename Derived, typename Traits>
template <typename... Args, typename>
inline
parametric_operation<Derived, Traits>::parametric_operation(Args&&... args)
	: m_signature(parameters_type(std::forward<Args>(args)...))
{
	m_signature.check_parameters(
		basic_operation<Derived, Traits>::get_static_descriptor()
	);
}

template <typename Derived, typename Traits>
inline
const typename parametric_operation<Derived, Traits>::parameters_type&
parametric_operation<Derived, Traits>::get_parameters() const noexcept
{
	return m_signature.get_parameters();
}

template <typename Derived, typename Traits>
inline
const operation_signature&
parametric_operation<Derived, Traits>::get_signature() const noexcept
{
	return m_signature;
}

} // namespace rexlib
