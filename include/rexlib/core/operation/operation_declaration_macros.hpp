// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/parametric_operation.hpp>
#include <rexlib/core/operation/trivial_operation.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

/**
 * @brief Build an operand name list for an operation declaration.
 *
 * Wrapping the names in their own parentheses is what lets them be passed
 * to the declaration macros as a single argument.
 */
#define REXLIB_OPERANDS(...) \
	::rexlib::make_operand_names(__VA_ARGS__)

/**
 * @brief Members every operation traits type declares.
 *
 * Shared by the traits declaration macros, which only add to it.
 */
#define REXLIB_DETAIL_OPERATION_TRAITS_MEMBERS( \
	op, outputs, inputs, shape, ... \
) \
	using shape_rule = shape; \
	using type_rule = __VA_ARGS__; \
	\
	static const char* get_name() noexcept \
	{ \
		return #op; \
	} \
	\
	static const auto& get_output_names() noexcept \
	{ \
		static REXLIB_CONST_CONSTEXPR auto names = outputs; \
		return names; \
	} \
	\
	static const auto& get_input_names() noexcept \
	{ \
		static REXLIB_CONST_CONSTEXPR auto names = inputs; \
		return names; \
	}

/**
 * @brief Declare the static description of an operation.
 *
 * Generates a traits type named `<op>_operation_traits`, deriving the
 * operation name from @p op itself so that the identifier and the name
 * cannot disagree.
 *
 * Use this directly, together with a hand written class, for an operation
 * that carries values its contract does not read. Use
 * @ref REXLIB_DECLARE_OPERATION for the usual case.
 *
 * @param op The operation verb, such as `add`. The generated class is named
 * `<op>_operation`.
 * @param component_traits A type providing `get_component()`, naming the
 * component the operation belongs to.
 * @param outputs Output operand names, as @ref REXLIB_OPERANDS.
 * @param inputs Input operand names, as @ref REXLIB_OPERANDS.
 * @param shape The operand shape rule, written without commas.
 * @param ... The operand typing rule. It comes last so that the commas in
 * its template arguments do not split the macro arguments.
 */
#define REXLIB_DECLARE_OPERATION_TRAITS( \
	op, component_traits, outputs, inputs, shape, ... \
) \
	struct op##_operation_traits \
		: component_traits \
	{ \
		REXLIB_DETAIL_OPERATION_TRAITS_MEMBERS( \
			op, outputs, inputs, shape, __VA_ARGS__ \
		) \
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
 * @param outputs Output operand names, as @ref REXLIB_OPERANDS.
 * @param inputs Input operand names, as @ref REXLIB_OPERANDS.
 * @param shape The operand shape rule, written without commas.
 * @param ... The operand typing rule.
 *
 * @see REXLIB_DECLARE_OPERATION_TRAITS
 */
#define REXLIB_DECLARE_OPERATION( \
	op, component_traits, outputs, inputs, shape, ... \
) \
	REXLIB_DECLARE_OPERATION_TRAITS( \
		op, component_traits, outputs, inputs, shape, __VA_ARGS__ \
	); \
	\
	REXLIB_BEGIN_TEMPLATE_BASE \
	class REXLIB_API op##_operation final \
		: public ::rexlib::trivial_operation< \
			op##_operation, \
			op##_operation_traits \
		> \
	{ \
	}; \
	REXLIB_END_TEMPLATE_BASE

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
