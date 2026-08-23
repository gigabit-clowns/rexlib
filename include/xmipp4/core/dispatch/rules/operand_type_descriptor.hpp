// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_pivot.hpp"
#include "operand_type_slot.hpp"

#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/numerical/numerical_type_domain.hpp>
#include <xmipp4/core/span.hpp>

#include <array>
#include <cstddef>
#include <functional>

namespace rex
{

/**
 * @brief Type erased form of an operand_type_pivot.
 *
 * A typing rule is a compile time declaration, but interpreting it at
 * runtime does not need any of its types. Lowering it to a table of plain
 * descriptors means the interpreter is one ordinary function rather than
 * one instantiation per rule, which matters when the catalog holds
 * hundreds of operations.
 *
 * @see operand_type_pivot
 */
class pivot_descriptor
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
	pivot_descriptor(
		pivot_source source,
		std::size_t index,
		std::ptrdiff_t default_pivot,
		const numerical_type_domain &domain
	) noexcept;

	/**
	 * @brief Get where the pivot value comes from.
	 *
	 * @return pivot_source The source.
	 */
	pivot_source get_source() const noexcept;

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
	pivot_source m_source;
	std::size_t m_index;
	std::ptrdiff_t m_default_pivot;

	// A reference rather than a pointer, so that a descriptor cannot exist
	// without a domain, and wrapped so that it stays assignable, which
	// holding descriptors in an array and handing out spans of them needs.
	std::reference_wrapper<const numerical_type_domain> m_domain;
};

/**
 * @brief Type erased form of an operand_type_slot.
 *
 * @see operand_type_slot
 */
class slot_descriptor
{
public:
	/**
	 * @brief Value level half of a transform.
	 */
	using transform_function = numerical_type (*)(numerical_type);

	/**
	 * @brief Construct a slot descriptor.
	 *
	 * @param pivot_index Index of the pivot the operand derives from.
	 * @param apply Value level half of the transform. Must not be null.
	 * @param transform_domain Set of pivot types the transform is defined
	 * on. It must outlive the descriptor.
	 * @param relation Fragment naming the mapping, for diagnostics. Must
	 * not be null and must outlive the descriptor.
	 */
	slot_descriptor(
		std::size_t pivot_index,
		transform_function apply,
		const numerical_type_domain &transform_domain,
		const char *relation
	) noexcept;

	/**
	 * @brief Get the index of the pivot the operand derives from.
	 *
	 * @return std::size_t The index.
	 */
	std::size_t get_pivot_index() const noexcept;

	/**
	 * @brief Apply the transform to a pivot value.
	 *
	 * @param type The pivot value. Must belong to the transform's domain.
	 * @return numerical_type The operand type.
	 */
	numerical_type apply(numerical_type type) const noexcept;

	/**
	 * @brief Get the set of pivot types the transform is defined on.
	 *
	 * @return const numerical_type_domain& The domain.
	 */
	const numerical_type_domain& get_transform_domain() const noexcept;

	/**
	 * @brief Get the fragment naming the mapping.
	 *
	 * @return const char* The fragment.
	 */
	const char* get_relation() const noexcept;

private:
	std::size_t m_pivot_index;
	transform_function m_apply;
	std::reference_wrapper<const numerical_type_domain> m_transform_domain;
	const char *m_relation;
};

/**
 * @brief Lower a rule's pivot list into a table of descriptors.
 *
 * @tparam PivotList type_list of operand_type_pivot declarations.
 */
template <typename PivotList>
struct pivot_descriptor_table;

template <typename... Pivots>
struct pivot_descriptor_table<type_list<Pivots...>>
{
	static span<const pivot_descriptor> get() noexcept
	{
		static const std::array<pivot_descriptor, sizeof...(Pivots)> table =
		{{
			pivot_descriptor(
				Pivots::source,
				Pivots::index,
				Pivots::default_pivot,
				Pivots::domain_type::get()
			)...
		}};

		return make_span(table.data(), table.size());
	}
};

/**
 * @brief Lower a rule's slot list into a table of descriptors.
 *
 * @tparam SlotList type_list of operand_type_slot declarations.
 */
template <typename SlotList>
struct slot_descriptor_table;

template <>
struct slot_descriptor_table<type_list<>>
{
	static span<const slot_descriptor> get() noexcept
	{
		return span<const slot_descriptor>();
	}
};

template <typename... Slots>
struct slot_descriptor_table<type_list<Slots...>>
{
	static span<const slot_descriptor> get() noexcept
	{
		static const std::array<slot_descriptor, sizeof...(Slots)> table =
		{{
			slot_descriptor(
				Slots::pivot_index,
				&Slots::transform_type::apply,
				Slots::transform_type::domain_type::get(),
				Slots::transform_type::get_relation()
			)...
		}};

		return make_span(table.data(), table.size());
	}
};

} // namespace rex

#include "operand_type_descriptor.inl"
