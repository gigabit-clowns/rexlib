// SPDX-License-Identifier: GPL-3.0-only

#include "basic_operation.hpp"

namespace rex
{

template <typename Derived, typename Traits>
inline
const operation_descriptor&
basic_operation<Derived, Traits>::get_static_descriptor() noexcept
{
	static const operation_descriptor descriptor = make_operation_descriptor(
		Traits::get_component(),
		Traits::get_name(),
		Traits::get_output_names(),
		Traits::get_input_names()
	);
	return descriptor;
}

template <typename Derived, typename Traits>
inline
const operation_descriptor&
basic_operation<Derived, Traits>::get_descriptor() const noexcept
{
	return get_static_descriptor();
}

template <typename Derived, typename Traits>
inline
operation_id basic_operation<Derived, Traits>::get_id() const noexcept
{
	return operation_id::of<Derived>();
}

template <typename Derived, typename Traits>
inline
std::string basic_operation<Derived, Traits>::get_name() const
{
	return Traits::get_name();
}

template <typename Derived, typename Traits>
inline
operation_arity
basic_operation<Derived, Traits>::get_arity() const noexcept
{
	return get_static_descriptor().get_arity();
}

template <typename Derived, typename Traits>
inline
const operation_shape_policy&
trivial_operation<Derived, Traits>
::get_operation_shape_policy() const noexcept
{
	return Traits::shape_policy_type::get();
}

template <typename Derived, typename Traits>
inline
const operation_data_type_policy&
trivial_operation<Derived, Traits>
::get_operation_data_type_policy() const noexcept
{
	return rule_data_type_policy<type_rule>::get();
}

} // namespace rex
