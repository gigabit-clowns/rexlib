// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot_source.hpp>

#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include <cstddef>
#include <functional>

namespace rexlib
{

/**
 * @brief Type erased form of a type_pivot.
 *
 * @see type_pivot
 */
class type_pivot_descriptor
{
public:
	/**
	 * @brief Construct a pivot descriptor.
	 *
	 * @param source Where the pivot value comes from.
	 * @param index Index of the operand, or of the operation supplied
	 * value.
	 * @param default_pivot Index of the pivot whose value is adopted by
	 * default, or -1 for none.
	 * @param domain Set of types the pivot may take. It must outlive the
	 * descriptor, which the domain tags guarantee by holding it in static
	 * storage.
	 */
	type_pivot_descriptor(
		type_pivot_source source,
		std::size_t index,
		std::ptrdiff_t default_pivot,
		const numerical_type_domain &domain
	) noexcept;

	/**
	 * @brief Get where the pivot value comes from.
	 *
	 * @return type_pivot_source The source.
	 */
	type_pivot_source get_source() const noexcept;

	/**
	 * @brief Get the index of the operand supplying the pivot.
	 *
	 * @return std::size_t The index.
	 */
	std::size_t get_index() const noexcept;

	/**
	 * @brief Get the pivot whose value this one adopts by default.
	 *
	 * @return std::ptrdiff_t The pivot index, or -1 for none.
	 */
	std::ptrdiff_t get_default_pivot() const noexcept;

	/**
	 * @brief Get the set of types the pivot may take.
	 *
	 * @return const numerical_type_domain& The domain.
	 */
	const numerical_type_domain& get_domain() const noexcept;

private:
	type_pivot_source m_source;
	std::size_t m_index;
	std::ptrdiff_t m_default_pivot;
	std::reference_wrapper<const numerical_type_domain> m_domain;
};

} // namespace rexlib

#include "type_pivot_descriptor.inl"
