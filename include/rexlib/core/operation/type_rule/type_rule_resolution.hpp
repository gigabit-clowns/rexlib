// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot.hpp>
#include <rexlib/core/operation/type_rule/type_rule_status.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <array>
#include <cstddef>

namespace rexlib
{

/**
 * @brief Resolved pivot values of a typing rule, or the reason it failed.
 *
 * Allocation free and trivially copyable. A failure is reported as a
 * status, not thrown.
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
	 * Unlike @ref failure, the pivots resolved so far are preserved.
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
	 * max_type_pivot_count.
	 * @param type The pivot value.
	 */
	REXLIB_API
	void set_pivot(std::size_t index, numerical_type type) noexcept;

	/**
	 * @brief Set the number of pivots the resolution holds.
	 *
	 * @param count The pivot count. Must not exceed
	 * max_type_pivot_count.
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
	std::array<numerical_type, max_type_pivot_count> m_pivots;
	std::size_t m_pivot_count;
	type_rule_status m_status;
	std::size_t m_operand_index;
	bool m_output_operand;
	numerical_type m_offending_type;
};

} // namespace rexlib
