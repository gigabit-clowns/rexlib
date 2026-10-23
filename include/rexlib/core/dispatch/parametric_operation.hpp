// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "basic_operation.hpp"

#include "rules/rule_operation_contract.hpp"

#include "../platform/dynamic_shared_object.h"

#include <type_traits>
#include <utility>

namespace rexlib
{

/**
 * @brief CRTP base for operations whose contract depends on parameters.
 *
 * Owns a rule_operation_contract holding the parameters of the operation
 * and forwards every constructor argument to them, so an operation whose
 * contract reads its parameters, such as a reduction over a set of axes,
 * needs no body of its own. The parameters are checked against the rules
 * on construction.
 *
 * Only parameters the contract reads are covered. An operation carrying a
 * value its contract does not read declares that value itself, as
 * fill_operation does.
 *
 * The contract is not copyable, so the operation is not either.
 *
 * @tparam Derived The concrete operation type.
 * @tparam Traits The static description. In addition to what
 * basic_operation requires, it must provide `using parameters_type = ...`
 * naming the type of the parameters, and its rules must read them.
 *
 * @see basic_operation
 * @see trivial_operation
 * @see rule_operation_contract
 */
template <typename Derived, typename Traits>
class parametric_operation
	: public basic_operation<Derived, Traits>
{
public:
	using typename basic_operation<Derived, Traits>::shape_rule;
	using typename basic_operation<Derived, Traits>::type_rule;
	using parameters_type = typename Traits::parameters_type;
	using contract_type = rule_operation_contract<
		shape_rule,
		type_rule,
		parameters_type
	>;

	static_assert(
		shape_rule::requires_parameters ||
		detail::rule_has_operation_pivot<
			typename type_rule::pivot_list
		>::value,
		"The rules of a parametric operation read nothing from its "
		"parameters. Declare it as a trivial operation instead."
	);

	/**
	 * @brief Construct the operation by constructing its parameters.
	 *
	 * Constrained on the parameters being constructible from @p args, so
	 * that a wrong argument list is reported as no matching constructor
	 * rather than as an error inside the parameters, and so that this
	 * constructor does not take part in copy initialization.
	 *
	 * @tparam Args Constructor argument types of the parameters.
	 * @param args Forwarded to the parameters.
	 *
	 * @throws std::invalid_argument When the rules do not admit the
	 * parameters.
	 */
	template <
		typename... Args,
		typename = typename std::enable_if<
			std::is_constructible<parameters_type, Args...>::value
		>::type
	>
	explicit parametric_operation(Args&&... args);

	/**
	 * @brief Get the parameters of the operation.
	 *
	 * How a program builder reads them back. A builder is instantiated on
	 * the operation type, so it recovers the operation with operation_cast
	 * and never has to downcast the erased contract.
	 *
	 * @return const parameters_type& The parameters.
	 */
	const parameters_type& get_parameters() const noexcept;

	const operation_contract& get_contract() const noexcept override;

private:
	contract_type m_contract;
};

} // namespace rexlib

/**
 * @brief Declare the static description of an operation with parameters.
 *
 * As @ref REXLIB_DECLARE_OPERATION_TRAITS, adding the type of the
 * parameters the rules of the operation read.
 *
 * @param op The operation verb, such as `sum`.
 * @param component_traits A type providing `get_component()`.
 * @param outputs Output operand names, as @ref REXLIB_OPERANDS.
 * @param inputs Input operand names, as @ref REXLIB_OPERANDS.
 * @param shape The operand shape rule, written without commas.
 * @param parameters The type of the parameters.
 * @param ... The operand typing rule.
 */
#define REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS( \
	op, component_traits, outputs, inputs, shape, parameters, ... \
) \
	struct op##_operation_traits \
		: component_traits \
	{ \
		using parameters_type = parameters; \
		\
		REXLIB_DETAIL_OPERATION_TRAITS_MEMBERS( \
			op, outputs, inputs, shape, __VA_ARGS__ \
		) \
	}

/**
 * @brief Declare an operation with parameters.
 *
 * Generates `<op>_operation_traits` and an exported `<op>_operation` class
 * whose constructors are those of its parameters. As with
 * @ref REXLIB_DECLARE_OPERATION the class is a plain named type rather than
 * an alias of a template specialization, so that its identity is well
 * defined across shared object boundaries.
 *
 * Use @ref REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS with a hand written
 * class instead when the operation carries values its contract does not
 * read.
 *
 * @param op The operation verb, such as `sum`.
 * @param component_traits A type providing `get_component()`.
 * @param outputs Output operand names, as @ref REXLIB_OPERANDS.
 * @param inputs Input operand names, as @ref REXLIB_OPERANDS.
 * @param shape The operand shape rule, written without commas.
 * @param parameters The type of the parameters. Its constructors become the
 * constructors of the operation.
 * @param ... The operand typing rule.
 *
 * @see REXLIB_DECLARE_OPERATION
 * @see parametric_operation
 */
#define REXLIB_DECLARE_PARAMETRIC_OPERATION( \
	op, component_traits, outputs, inputs, shape, parameters, ... \
) \
	REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS( \
		op, component_traits, outputs, inputs, shape, parameters, \
		__VA_ARGS__ \
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
