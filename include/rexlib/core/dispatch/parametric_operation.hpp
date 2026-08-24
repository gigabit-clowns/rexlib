// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "basic_operation.hpp"

#include "../platform/dynamic_shared_object.h"

#include <type_traits>
#include <utility>

namespace rexlib
{

/**
 * @brief CRTP base for operations owning a stateful shape policy.
 *
 * Holds the shape policy named by @p Traits by value and forwards every
 * constructor argument to it, so an operation whose shape deduction is
 * parameterised, such as a reduction over a set of axes, needs no body of
 * its own.
 *
 * This is what sets it apart from trivial_operation: the policy is a member
 * rather than a process wide singleton, so two instances of the same
 * operation may deduce different shapes. The data type policy is still the
 * shared one derived from the typing rule, because a shape parameter never
 * changes the typing.
 *
 * Only parameters belonging to the shape policy are covered. An operation
 * carrying a parameter that does not affect its shape keeps a stateless
 * policy and declares that parameter itself, as fill_operation does.
 *
 * Both policy interfaces are non copyable, so the policy is constructed in
 * place and never reassigned, and the operation is non copyable in turn.
 *
 * @tparam Derived The concrete operation type.
 * @tparam Traits The static description. In addition to what
 * basic_operation requires, it must provide `using shape_policy_type = ...`
 * naming a concrete operation_shape_policy. Unlike trivial_operation, that
 * policy needs no static `get()`.
 *
 * @see basic_operation
 * @see trivial_operation
 */
template <typename Derived, typename Traits>
class parametric_operation
	: public basic_operation<Derived, Traits>
{
public:
	using typename basic_operation<Derived, Traits>::type_rule;
	using shape_policy_type = typename Traits::shape_policy_type;

	/**
	 * @brief Construct the operation by constructing its shape policy.
	 *
	 * Constrained on the policy being constructible from @p args, so that a
	 * wrong argument list is reported as no matching constructor rather
	 * than as an error inside the policy, and so that this constructor does
	 * not take part in copy initialization.
	 *
	 * @tparam Args Constructor argument types of the shape policy.
	 * @param args Forwarded to the shape policy.
	 */
	template <
		typename... Args,
		typename = typename std::enable_if<
			std::is_constructible<
				typename Traits::shape_policy_type,
				Args...
			>::value
		>::type
	>
	explicit parametric_operation(Args&&... args);

	/**
	 * @brief Get the shape policy with its concrete type.
	 *
	 * How a program builder reads back the parameters of the operation. A
	 * builder is instantiated on the operation type, so it recovers the
	 * operation with operation_cast and asks it for the policy, and never
	 * has to downcast the erased base.
	 *
	 * @return const shape_policy_type& The shape policy.
	 */
	const shape_policy_type& get_shape_policy() const noexcept;

	const operation_shape_policy&
	get_operation_shape_policy() const noexcept override;

	const operation_data_type_policy&
	get_operation_data_type_policy() const noexcept override;

private:
	shape_policy_type m_shape_policy;
};

} // namespace rexlib

/**
 * @brief Declare an operation owning a stateful shape policy.
 *
 * Generates `<op>_operation_traits` and an exported `<op>_operation` class
 * whose constructors are those of its shape policy. As with
 * @ref REXLIB_DECLARE_OPERATION the class is a plain named type rather than
 * an alias of a template specialization, so that its identity is well
 * defined across shared object boundaries.
 *
 * Use @ref REXLIB_DECLARE_OPERATION_TRAITS with a hand written class
 * instead when the parameters need validating, which an inherited
 * constructor has no place to do.
 *
 * @param op The operation verb, such as `sum`.
 * @param component_traits A type providing `get_component()`.
 * @param outputs Output operand names, as @ref REXLIB_OPERANDS.
 * @param inputs Input operand names, as @ref REXLIB_OPERANDS.
 * @param shape_policy The stateful shape policy type. Its constructors
 * become the constructors of the operation.
 * @param ... The operand typing rule.
 *
 * @see REXLIB_DECLARE_OPERATION
 * @see parametric_operation
 */
#define REXLIB_DECLARE_PARAMETRIC_OPERATION( \
	op, component_traits, outputs, inputs, shape_policy, ... \
) \
	REXLIB_DECLARE_OPERATION_TRAITS( \
		op, component_traits, outputs, inputs, shape_policy, __VA_ARGS__ \
	); \
	\
	REXLIB_BEGIN_TEMPLATE_BASE \
	class REXLIB_API op##_operation final \
		: public ::rexlib::parametric_operation< \
			op##_operation, \
			op##_operation_traits \
		> \
	{ \
	public: \
		using ::rexlib::parametric_operation< \
			op##_operation, \
			op##_operation_traits \
		>::parametric_operation; \
	}; \
	REXLIB_END_TEMPLATE_BASE

#include "parametric_operation.inl"
