// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_pivot.hpp"

#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <array>
#include <cstddef>

namespace xmipp4
{

/**
 * @brief Outcome of resolving a typing rule against concrete operand types.
 */
enum class type_rule_status
{
	/**
	 * @brief The rule resolved successfully.
	 */
	ok,

	/**
	 * @brief A pivot took a value that is not a concrete numerical type.
	 */
	invalid_type,

	/**
	 * @brief A pivot took a value outside the domain the rule declares.
	 */
	domain_violation,

	/**
	 * @brief A transform is not defined on the value its pivot took.
	 */
	undefined_transform,

	/**
	 * @brief An operand type disagrees with the slot binding it.
	 */
	slot_mismatch,

	/**
	 * @brief A free pivot could not be determined from the operands.
	 */
	unresolved_pivot
};

/**
 * @brief Get a human readable representation of a rule status.
 *
 * @param status The status.
 * @return const char* The representation.
 */
REXLIB_API
const char* to_string(type_rule_status status) noexcept;

/**
 * @brief Resolved pivot values of a typing rule, or the reason it failed.
 *
 * Allocation free and trivially copyable, so that resolving a rule stays
 * cheap enough to run on every dispatch and, crucially, cheap enough for a
 * backend to run when merely asked whether it supports a launch
 * configuration. Failures are reported rather than thrown for that reason:
 * asking is not an error.
 *
 * @see operand_type_rule
 */
class type_rule_resolution
{
public:
	/**
	 * @brief Construct a successful resolution with no pivot resolved yet.
	 */
	REXLIB_API
	type_rule_resolution() noexcept;

	/**
	 * @brief Construct a failed resolution.
	 *
	 * @param status The reason of the failure. Must not be
	 * type_rule_status::ok.
	 * @param operand_index Index of the operand the failure is attributed
	 * to.
	 * @param output_operand Whether @p operand_index refers to an output
	 * operand.
	 * @param type The offending type.
	 * @return type_rule_resolution The failed resolution.
	 */
	REXLIB_API
	static type_rule_resolution failure(
		type_rule_status status,
		std::size_t operand_index,
		bool output_operand,
		numerical_type type
	) noexcept;

	/**
	 * @brief Derive a failed resolution from this one.
	 *
	 * Unlike @ref failure, the pivots resolved so far are preserved, which
	 * is what lets a diagnostic state the type the rule expected and not
	 * merely the one it got.
	 *
	 * @param status The reason of the failure. Must not be
	 * type_rule_status::ok.
	 * @param operand_index Index of the operand the failure is attributed
	 * to.
	 * @param output_operand Whether @p operand_index refers to an output
	 * operand.
	 * @param type The offending type.
	 * @return type_rule_resolution The failed resolution.
	 */
	REXLIB_API
	type_rule_resolution fail(
		type_rule_status status,
		std::size_t operand_index,
		bool output_operand,
		numerical_type type
	) const noexcept;

	/**
	 * @brief Get the outcome of the resolution.
	 *
	 * @return type_rule_status The status.
	 */
	REXLIB_API
	type_rule_status get_status() const noexcept;

	/**
	 * @brief Check whether the resolution succeeded.
	 *
	 * @return true The rule resolved successfully.
	 * @return false The rule could not be resolved.
	 */
	REXLIB_API
	explicit operator bool() const noexcept;

	/**
	 * @brief Get the number of resolved pivots.
	 *
	 * @return std::size_t The pivot count.
	 */
	REXLIB_API
	std::size_t get_pivot_count() const noexcept;

	/**
	 * @brief Get the value of a resolved pivot.
	 *
	 * @param index Index of the pivot. Must be less than the pivot count.
	 * @return numerical_type The pivot value. May be
	 * numerical_type::unknown when the pivot is free and was not bound.
	 */
	REXLIB_API
	numerical_type get_pivot(std::size_t index) const noexcept;

	/**
	 * @brief Set the value of a pivot.
	 *
	 * @param index Index of the pivot. Must be less than
	 * max_operand_type_pivot_count.
	 * @param type The pivot value.
	 */
	REXLIB_API
	void set_pivot(std::size_t index, numerical_type type) noexcept;

	/**
	 * @brief Set the number of pivots the resolution holds.
	 *
	 * @param count The pivot count. Must not exceed
	 * max_operand_type_pivot_count.
	 */
	REXLIB_API
	void set_pivot_count(std::size_t count) noexcept;

	/**
	 * @brief Get the index of the operand a failure is attributed to.
	 *
	 * @return std::size_t The operand index.
	 */
	REXLIB_API
	std::size_t get_operand_index() const noexcept;

	/**
	 * @brief Check whether the failing operand is an output.
	 *
	 * @return true The operand is an output operand.
	 * @return false The operand is an input operand.
	 */
	REXLIB_API
	bool is_output_operand() const noexcept;

	/**
	 * @brief Get the type that caused the failure.
	 *
	 * @return numerical_type The offending type.
	 */
	REXLIB_API
	numerical_type get_offending_type() const noexcept;

private:
	std::array<numerical_type, max_operand_type_pivot_count> m_pivots;
	std::size_t m_pivot_count;
	type_rule_status m_status;
	std::size_t m_operand_index;
	bool m_output_operand;
	numerical_type m_offending_type;
};

} // namespace xmipp4
