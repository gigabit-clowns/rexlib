// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/meta/type_list.hpp>
#include <rex/core/platform/cpp_attributes.hpp>
#include <rex/backends/cpu/program.hpp>
#include <rex/backends/cpu/thread_pool.hpp>

#include <memory>
#include <array>
#include <type_traits>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Adapts a strongly typed functor to the @ref program interface.
 *
 * The @ref program interface is type erased: its @ref program::execute
 * method receives its operands as spans of @ref buffer handles, without any
 * static knowledge of the element types they hold. This class bridges that gap
 * by pairing a functor with compile time descriptions of its operand types.
 *
 * On @ref execute, the underlying storage of each operand buffer is obtained
 * through @ref buffer::get_host_ptr and reinterpreted according to the matching
 * entry in @p Outputs, @p Inputs and @p Scratches. The wrapped functor is then
 * invoked with three tuples of pointers to the operands' element types:
 * - Outputs and scratches are passed as pointers to mutable elements
 *   (`T *`).
 * - Inputs are passed as pointers to const elements (`const T *`).
 *
 * Because @ref buffer::get_host_ptr is used to access the storage, every
 * operand buffer must be host accessible. @ref execute throws if any of the
 * supplied buffers cannot provide a host pointer.
 *
 * A type list entry may be `void`, in which case the corresponding operand is
 * left type erased and forwarded to the functor as a `void *` (or `const void
 * *` for inputs) instead of being reinterpreted to a concrete element type.
 * This is useful for operands whose element type is not known at compile time
 * or is irrelevant to the operation.
 *
 * The order of the types within each list must match the order of the
 * corresponding buffers supplied to @ref execute, and the number of types in
 * each list must match the number of buffers in the respective span.
 *
 * @tparam F The functor to be wrapped. Must be callable with three tuples of
 * pointers to the operands' element types, in the order (outputs, inputs,
 * scratches), where output and scratch pointers are non-const and input
 * pointers are const-qualified, followed by the @ref thread_pool the program
 * was handed.
 * @tparam Outputs List of output element types, one per output buffer. Must be
 * a specialization of @ref rexlib::type_list.
 * @tparam Inputs List of input element types, one per input buffer. Must be a
 * specialization of @ref rexlib::type_list.
 * @tparam Scratches List of scratch (workspace) element types, one per scratch
 * buffer. Must be a specialization of @ref rexlib::type_list.
 *
 * @see program
 * @see make_functor_program
 */
template <typename F, typename Outputs, typename Inputs, typename Scratches>
class functor_program final
	: public program
{
public:
	using functor_type = F;
	using output_types = Outputs;
	using input_types = Inputs;
	using scratch_types = Scratches;
	using scratch_requirement_array_type = std::array<
		program_scratch_requirement, 
		type_list_size<scratch_types>::value
	>;

	/**
	 * @brief Construct a program that requires no scratch buffers.
	 *
	 * The functor is stored by value inside the wrapper, copied or moved from
	 * @p functor depending on the value category of the argument.
	 *
	 * This overload only participates in overload resolution when
	 * @ref scratch_types is empty; programs with scratch types must be
	 * constructed through the overload that also takes the scratch
	 * requirements.
	 *
	 * @param functor The functor to be invoked on @ref execute. Defaults to a
	 * value-initialized @ref functor_type, which is convenient for stateless
	 * functors.
	 */
	template <
		typename S = scratch_types,
		typename = std::enable_if_t<type_list_size<S>::value == 0>
	>
	explicit functor_program(functor_type functor = {});

	/**
	 * @brief Construct a program from a functor and its scratch requirements.
	 *
	 * The functor is stored by value inside the wrapper, copied or moved from
	 * @p functor depending on the value category of the argument.
	 *
	 * Exactly one @ref program_scratch_requirement must be provided for each
	 * type in @ref scratch_types, in the same order. When @ref scratch_types is
	 * empty, @p scratch_requirements is an empty array and this behaves like
	 * the scratch-free overload.
	 *
	 * @param functor The functor to be invoked on @ref execute.
	 * @param scratch_requirements The size and alignment requirements of each
	 * scratch buffer, surfaced through @ref get_scratch_requirements.
	 */
	functor_program(
		functor_type functor,
		scratch_requirement_array_type scratch_requirements
	);

	~functor_program() override = default;

	void execute(
		span<const std::shared_ptr<buffer>> output_operands,
		span<const std::shared_ptr<const buffer>> input_operands,
		span<const std::shared_ptr<buffer>> scratch_operands,
		thread_pool &pool
	) const override;

	span<const program_scratch_requirement> 
	get_scratch_requirements() const override;

private:
	REXLIB_NO_UNIQUE_ADDRESS 
	functor_type m_function;

	REXLIB_NO_UNIQUE_ADDRESS 
	scratch_requirement_array_type m_scratch_requirements;

	template<
		std::size_t... OutputIs,
		std::size_t... InputIs,
		std::size_t... ScratchIs
	>
	void execute_wrapper(
		span<const std::shared_ptr<buffer>> output_operands,
		span<const std::shared_ptr<const buffer>> input_operands,
		span<const std::shared_ptr<buffer>> scratch_operands,
		thread_pool &pool,
		std::index_sequence<OutputIs...>,
		std::index_sequence<InputIs...>,
		std::index_sequence<ScratchIs...>
	) const;
};

/**
 * @brief Create a @ref functor_program that requires no scratch buffers.
 *
 * @tparam F Functor type, deduced from @p fun.
 * @tparam Outputs Output type list, deduced from @p output_types.
 * @tparam Inputs Input type list, deduced from @p input_types.
 * @param fun The functor to be wrapped.
 * @param output_types A @ref rexlib::type_list describing the output operands.
 * @param input_types A @ref rexlib::type_list describing the input operands.
 * @return The wrapped program, held through a `std::shared_ptr<program>`.
 */
template <typename F, typename Outputs, typename Inputs>
std::shared_ptr<program> make_functor_program(
	F &&fun,
	Outputs output_types,
	Inputs input_types
);

/**
 * @brief Create a @ref functor_program with scratch buffers.
 *
 * The number of entries in @p scratch_requirements must equal the number of
 * types in @p scratch_types; this is enforced at compile time by the array
 * size.
 *
 * @tparam F Functor type, deduced from @p fun.
 * @tparam Outputs Output type list, deduced from @p output_types.
 * @tparam Inputs Input type list, deduced from @p input_types.
 * @tparam Scratches Scratch type list, deduced from @p scratch_types.
 * @param fun The functor to be wrapped.
 * @param output_types A @ref rexlib::type_list describing the output operands.
 * @param input_types A @ref rexlib::type_list describing the input operands.
 * @param scratch_types A @ref rexlib::type_list describing the scratch operands.
 * @param scratch_requirements One requirement per scratch type, in order.
 * @return The wrapped program, held through a `std::shared_ptr<program>`.
 */
template <typename F, typename Outputs, typename Inputs, typename Scratches>
std::shared_ptr<program> make_functor_program(
	F &&fun,
	Outputs output_types,
	Inputs input_types,
	Scratches scratch_types,
	std::array<program_scratch_requirement, type_list_size<Scratches>::value>
		scratch_requirements
);

} // namespace cpu
} // namespace rexlib

#include "functor_program.inl"
