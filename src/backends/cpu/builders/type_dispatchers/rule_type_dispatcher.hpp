// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_descriptor.hpp>
#include <rexlib/core/dispatch/rules/operand_type_descriptor.hpp>
#include <rexlib/core/dispatch/rules/operand_type_resolution.hpp>
#include <rexlib/core/dispatch/rules/rule_operand_types.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace rexlib
{

class program;

namespace cpu
{

/**
 * @brief Support predicate accepting every pivot type combination.
 *
 * Default predicate of rule_type_dispatcher. A backend only needs its own
 * when it cannot execute something the operation itself allows.
 */
template <typename... Types>
struct rule_always_supported : std::true_type {};

/**
 * @brief Type dispatcher driven by an operation's typing rule.
 *
 * Resolves the rule's pivots from the operand data types, dispatches over
 * the pivots alone, and derives each operand's element type by applying its
 * slot transform. Dispatching pivots rather than operands is what bounds
 * the number of instantiations by the pivot count instead of the operand
 * count: a rule with one free element type costs 16 instantiations no
 * matter how many operands it has.
 *
 * The rule is the operation's own declaration, so this dispatcher restates
 * none of it. @p Support is for the separate question of what this backend
 * can execute, which is legitimately narrower than what the operation
 * allows.
 *
 * @tparam Rule The operand_type_rule to interpret.
 * @tparam Support Predicate instantiated as Support<Pivots...> over the
 * resolved pivot element types. When it rejects a combination the factory
 * is not invoked, so its body is never instantiated for that combination.
 */
template <
	typename Rule,
	template <typename...> class Support = rule_always_supported
>
class rule_type_dispatcher
{
public:
	/**
	 * @brief Check whether this backend can serve the given operand types.
	 *
	 * Answers without throwing, so that a builder can report itself
	 * unsupported and let the program manager try another backend, rather
	 * than an unsupported type combination surfacing as an error from deep
	 * inside program construction.
	 *
	 * @param output_types Runtime data type of each output operand.
	 * @param input_types Runtime data type of each input operand.
	 * @param operation_pivots Pivot values supplied by the operation, for
	 * rules that take one. Empty otherwise.
	 * @return true The operand types resolve and are supported.
	 * @return false They do not.
	 */
	static bool is_supported(
		span<const numerical_type> output_types,
		span<const numerical_type> input_types,
		span<const numerical_type> operation_pivots =
			span<const numerical_type>()
	) noexcept;

	/**
	 * @brief Resolve the operand element types and invoke the factory.
	 *
	 * @param descriptor Description of the operation, used to name it and
	 * its operands when reporting a rejection.
	 * @param factory Program factory invoked as factory(output_types,
	 * input_types), where both arguments are type_list-s of the resolved
	 * element types.
	 * @param output_types Runtime data type of each output operand.
	 * @param input_types Runtime data type of each input operand.
	 * @param operation_pivots Pivot values supplied by the operation.
	 * @return std::shared_ptr<rexlib::program> The program built by the
	 * factory.
	 * @throws std::invalid_argument When the operand types do not resolve
	 * against the rule, or the resolved pivots are rejected by @p Support.
	 */
	template <typename F, std::size_t OutputCount, std::size_t InputCount>
	static std::shared_ptr<rexlib::program> dispatch(
		const operation_descriptor &descriptor,
		F &&factory,
		const std::array<numerical_type, OutputCount> &output_types,
		const std::array<numerical_type, InputCount> &input_types,
		span<const numerical_type> operation_pivots =
			span<const numerical_type>()
	);

private:
	using pivot_indices = std::make_index_sequence<Rule::pivot_count>;

	/**
	 * @brief Whether a pivot combination should be instantiated at all.
	 *
	 * Combines what the operation admits with what this backend can
	 * execute. Named rather than spelled out where it is used, both because
	 * the two callers must agree and because MSVC fails to compile the
	 * expression when it is built inline from a generic lambda's pack.
	 *
	 * @tparam Pivots The candidate pivot element types.
	 */
	template <typename... Pivots>
	struct is_admissible
		: std::integral_constant<
			bool,
			rule_pivots_in_domain<
				typename Rule::pivot_list,
				type_list<Pivots...>
			>::value &&
			Support<Pivots...>::value
		>
	{
	};

	static type_rule_resolution resolve(
		span<const numerical_type> output_types,
		span<const numerical_type> input_types,
		span<const numerical_type> operation_pivots
	) noexcept;

	template <typename F, std::size_t... PivotIndices>
	static std::shared_ptr<rexlib::program> dispatch_pivots(
		F &&factory,
		const type_rule_resolution &resolution,
		std::index_sequence<PivotIndices...>
	);

	template <std::size_t... PivotIndices>
	static bool check_support(
		const type_rule_resolution &resolution,
		std::index_sequence<PivotIndices...>
	);

	template <typename F, typename Pivots>
	static std::shared_ptr<rexlib::program> invoke_if(
		std::true_type supported,
		F &&factory,
		Pivots pivot_types
	);

	template <typename F, typename Pivots>
	REXLIB_NORETURN
	static std::shared_ptr<rexlib::program> invoke_if(
		std::false_type supported,
		F &&factory,
		Pivots pivot_types
	);
};

} // namespace cpu
} // namespace rexlib

#include "rule_type_dispatcher.inl"
