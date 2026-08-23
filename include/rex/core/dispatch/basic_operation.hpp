// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_names.hpp"
#include "operation.hpp"
#include "operation_arity.hpp"
#include "operation_descriptor.hpp"

#include "rules/rule_operation_data_type_policy.hpp"

#include "../platform/constexpr.hpp"
#include "../platform/dynamic_shared_object.h"

#include <cstddef>
#include <string>

namespace rex
{

/**
 * @brief CRTP base supplying the static description of an operation.
 *
 * Implements everything an operation can state once and for all: its name,
 * its arity and the names of its operands, all read from @p Traits. Both
 * policy getters are left pure, so an operation that owns a stateful policy
 * derives from this one and the compiler enforces that it provides them.
 *
 * The operand counts are exposed as compile time constants, which is what a
 * backend builder needs to size its operand arrays. They come from the
 * operation's typing rule and are checked against the operand name list, so
 * the arity cannot drift from either.
 *
 * @tparam Derived The concrete operation type. It keeps the base class type
 * distinct per operation, so that two operations sharing a rule do not
 * share a base.
 * @tparam Traits The static description. It must provide:
 *   - `static const char* get_component() noexcept`
 *   - `static const char* get_name() noexcept`
 *   - `static const operand_names<N>& get_output_names() noexcept`
 *   - `static const operand_names<M>& get_input_names() noexcept`
 *   - `using type_rule = ...`
 *
 * @see trivial_operation
 * @see operand_type_rule
 */
template <typename Derived, typename Traits>
class basic_operation
	: public operation
{
public:
	using traits_type = Traits;
	using type_rule = typename Traits::type_rule;

	static REX_CONST_CONSTEXPR std::size_t output_operand_count =
		type_rule::output_count;
	static REX_CONST_CONSTEXPR std::size_t input_operand_count =
		type_rule::input_count;

	static_assert(
		output_operand_count == operand_names_count<
			decltype(Traits::get_output_names())
		>::value,
		"The operation declares a different number of output operand names "
		"than its typing rule has output slots."
	);

	static_assert(
		input_operand_count == operand_names_count<
			decltype(Traits::get_input_names())
		>::value,
		"The operation declares a different number of input operand names "
		"than its typing rule has input slots."
	);

	/**
	 * @brief Get the description shared by every instance.
	 *
	 * @return const operation_descriptor& The descriptor.
	 */
	static const operation_descriptor& get_static_descriptor() noexcept;

	/**
	 * @brief Get the description of this operation.
	 *
	 * @return const operation_descriptor& The descriptor.
	 */
	const operation_descriptor& get_descriptor() const noexcept override;

	/**
	 * @brief Get the identifier of @p Derived.
	 *
	 * Pinned to @p Derived rather than to the dynamic type, so that an
	 * operation specialising another still dispatches to the builders of
	 * the operation it specialises.
	 *
	 * @return operation_id The operation id.
	 */
	operation_id get_id() const noexcept override;

	std::string get_name() const override;

	operation_arity get_arity() const noexcept override;
};

/**
 * @brief CRTP base for operations that are trivially declared.
 *
 * Adds the shared policy accessors named by @p Traits, so that the body of
 * the derived class is usually empty.
 *
 * "Trivial" refers to the declaration and not to the C++ notion of a
 * trivial type: every operation has a virtual destructor, and the operation
 * may still carry parameters of its own. What is trivial is that nothing
 * about its dispatch has to be written by hand.
 *
 * @tparam Derived The concrete operation type.
 * @tparam Traits The static description. In addition to what
 * basic_operation requires, it must provide `using shape_policy_type = ...`
 * naming a type with a static `get()` returning an operation_shape_policy.
 * The data type policy is derived from the typing rule and needs no
 * declaration of its own.
 *
 * @see basic_operation
 */
template <typename Derived, typename Traits>
class trivial_operation
	: public basic_operation<Derived, Traits>
{
public:
	using typename basic_operation<Derived, Traits>::type_rule;

	const operation_shape_policy&
	get_operation_shape_policy() const noexcept override;

	const operation_data_type_policy&
	get_operation_data_type_policy() const noexcept override;
};

} // namespace rex

/**
 * @brief Build an operand name list for an operation declaration.
 *
 * Wrapping the names in their own parentheses is what lets them be passed
 * to the declaration macros as a single argument.
 */
#define REX_OPERANDS(...) \
	::rex::make_operand_names(__VA_ARGS__)

/**
 * @brief Declare the static description of an operation.
 *
 * Generates a traits type named `<op>_operation_traits`, deriving the
 * operation name from @p op itself so that the identifier and the name
 * cannot disagree.
 *
 * Use this directly, together with a hand written class, for an operation
 * that carries parameters or owns a stateful policy. Use
 * @ref REX_DECLARE_OPERATION for the usual case.
 *
 * @param op The operation verb, such as `add`. The generated class is named
 * `<op>_operation`. 
 * @param component_traits A type providing `get_component()`, naming the
 * component the operation belongs to.
 * @param outputs Output operand names, as @ref REX_OPERANDS.
 * @param inputs Input operand names, as @ref REX_OPERANDS.
 * @param shape_policy Type with a static `get()` returning the shape policy.
 * @param ... The operand typing rule. It comes last so that the commas in
 * its template arguments do not split the macro arguments.
 */
#define REX_DECLARE_OPERATION_TRAITS( \
	op, component_traits, outputs, inputs, shape_policy, ... \
) \
	struct op##_operation_traits \
		: component_traits \
	{ \
		using shape_policy_type = shape_policy; \
		using type_rule = __VA_ARGS__; \
		\
		static const char* get_name() noexcept \
		{ \
			return #op; \
		} \
		\
		static const auto& get_output_names() noexcept \
		{ \
			static REX_CONST_CONSTEXPR auto names = outputs; \
			return names; \
		} \
		\
		static const auto& get_input_names() noexcept \
		{ \
			static REX_CONST_CONSTEXPR auto names = inputs; \
			return names; \
		} \
	}

/**
 * @brief Declare an operation and its static description.
 *
 * Generates `<op>_operation_traits` and an exported `<op>_operation` class
 * with an empty body. The class is a plain named type rather than an alias
 * of a template specialization, so that its identity is well defined across
 * shared object boundaries.
 *
 * @param op The operation verb, such as `add`.
 * @param component_traits A type providing `get_component()`.
 * @param outputs Output operand names, as @ref REX_OPERANDS.
 * @param inputs Input operand names, as @ref REX_OPERANDS.
 * @param shape_policy Type with a static `get()` returning the shape policy.
 * @param ... The operand typing rule.
 *
 * @see REX_DECLARE_OPERATION_TRAITS
 */
#define REX_DECLARE_OPERATION( \
	op, component_traits, outputs, inputs, shape_policy, ... \
) \
	REX_DECLARE_OPERATION_TRAITS( \
		op, component_traits, outputs, inputs, shape_policy, __VA_ARGS__ \
	); \
	\
	REX_BEGIN_TEMPLATE_BASE \
	class REXLIB_API op##_operation final \
		: public ::rex::trivial_operation< \
			op##_operation, \
			op##_operation_traits \
		> \
	{ \
	}; \
	REX_END_TEMPLATE_BASE

#include "basic_operation.inl"
