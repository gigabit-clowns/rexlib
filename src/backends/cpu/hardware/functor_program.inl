// SPDX-License-Identifier: GPL-3.0-only

#include "functor_program.hpp"

#include <xmipp4/core/hardware/buffer.hpp>

#include <tuple>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <sstream>

namespace xmipp4
{
namespace cpu
{

namespace detail
{

/**
 * @brief Check that an operand group holds the expected number of buffers.
 *
 * @param actual Number of buffers supplied for the group.
 * @param expected Number of types declared for the group.
 * @param context Name of the operand group ("output", "input", "scratch"),
 * used to build the error message.
 * @throws std::invalid_argument if @p actual differs from @p expected.
 */
inline void check_operand_count(
	std::size_t actual,
	std::size_t expected,
	const char *context
)
{
	if (actual != expected)
	{
		std::ostringstream oss;
		oss << "functor_program: expected " << expected << " " << context
			<< " operand(s) but got " << actual;
		throw std::invalid_argument(oss.str());
	}
}

/**
 * @brief Obtain a typed, host-accessible pointer to a buffer's storage.
 *
 * @tparam T Element type to reinterpret the storage as. May be @c void to keep
 * the pointer type erased.
 * @param buf The buffer to access.
 * @param context Name of the operand group ("output", "input", "scratch"),
 * used to build the error message.
 * @param index Position of the buffer within its operand group, used to build
 * the error message.
 * @return Pointer to the buffer's storage as @c T*.
 * @throws std::invalid_argument if the buffer is not host accessible.
 */
template <typename T>
inline T* get_typed_host_ptr(
	buffer &buf,
	const char *context,
	std::size_t index
)
{
	void *ptr = buf.get_host_ptr();
	if (ptr == nullptr)
	{
		std::ostringstream oss;
		oss << "functor_program: " << context
			<< " operand buffer at index " << index
			<< " is not host accessible";
		throw std::invalid_argument(oss.str());
	}
	return static_cast<T*>(ptr);
}

/**
 * @brief Const overload of @ref get_typed_host_ptr.
 */
template <typename T>
inline const T* get_typed_host_ptr(
	const buffer &buf,
	const char *context,
	std::size_t index
)
{
	const void *ptr = buf.get_host_ptr();
	if (ptr == nullptr)
	{
		std::ostringstream oss;
		oss << "functor_program: " << context
			<< " operand buffer at index " << index
			<< " is not host accessible";
		throw std::invalid_argument(oss.str());
	}
	return static_cast<const T*>(ptr);
}

} // namespace detail


template <typename F, typename Outputs, typename Inputs, typename Scratches>
template <typename S, typename>
inline
functor_program<F, Outputs, Inputs, Scratches>::functor_program(
	functor_type functor
)
	: m_function(std::move(functor))
	, m_scratch_requirements{}
{
}

template <typename F, typename Outputs, typename Inputs, typename Scratches>
inline
functor_program<F, Outputs, Inputs, Scratches>::functor_program(
	functor_type functor,
	scratch_requirement_array_type scratch_requirements
)
	: m_function(std::move(functor))
	, m_scratch_requirements(std::move(scratch_requirements))
{
}

template <typename F, typename Outputs, typename Inputs, typename Scratches>
inline void functor_program<F, Outputs, Inputs, Scratches>::execute(
	span<const std::shared_ptr<buffer>> output_operands,
	span<const std::shared_ptr<const buffer>> input_operands,
	span<const std::shared_ptr<buffer>> scratch_operands,
	thread_pool &pool
) const
{
	execute_wrapper(
		output_operands,
		input_operands,
		scratch_operands,
		pool,
		std::make_index_sequence<type_list_size<output_types>::value>(),
		std::make_index_sequence<type_list_size<input_types>::value>(),
		std::make_index_sequence<type_list_size<scratch_types>::value>()
	);
}

template <typename F, typename Outputs, typename Inputs, typename Scratches>
inline span<const program_scratch_requirement>
functor_program<F, Outputs, Inputs, Scratches>
::get_scratch_requirements() const
{
	return span<const program_scratch_requirement>(
		m_scratch_requirements.data(),
		m_scratch_requirements.size()
	);
}

template <typename F, typename Outputs, typename Inputs, typename Scratches>
template <
	std::size_t... OutputIs,
	std::size_t... InputIs,
	std::size_t... ScratchIs
>
inline void functor_program<F, Outputs, Inputs, Scratches>::execute_wrapper(
	span<const std::shared_ptr<buffer>> output_operands,
	span<const std::shared_ptr<const buffer>> input_operands,
	span<const std::shared_ptr<buffer>> scratch_operands,
	thread_pool &pool,
	std::index_sequence<OutputIs...>,
	std::index_sequence<InputIs...>,
	std::index_sequence<ScratchIs...>
) const
{
	detail::check_operand_count(
		output_operands.size(),
		sizeof...(OutputIs),
		"output"
	);
	detail::check_operand_count(
		input_operands.size(),
		sizeof...(InputIs),
		"input"
	);
	detail::check_operand_count(
		scratch_operands.size(),
		sizeof...(ScratchIs),
		"scratch"
	);

	m_function(
		std::make_tuple(
			detail::get_typed_host_ptr<
				typename type_list_element<OutputIs, output_types>::type
			>(*output_operands[OutputIs], "output", OutputIs)...
		),
		std::make_tuple(
			detail::get_typed_host_ptr<
				typename type_list_element<InputIs, input_types>::type
			>(*input_operands[InputIs], "input", InputIs)...
		),
		std::make_tuple(
			detail::get_typed_host_ptr<
				typename type_list_element<ScratchIs, scratch_types>::type
			>(*scratch_operands[ScratchIs], "scratch", ScratchIs)...
		),
		pool
	);
}


template <typename F, typename Outputs, typename Inputs>
inline std::shared_ptr<program> make_functor_program(
	F &&fun,
	Outputs,
	Inputs
)
{
	using program_type = functor_program<
		std::decay_t<F>, Outputs, Inputs, type_list<>
	>;
	return std::make_shared<program_type>(std::forward<F>(fun));
}

template <typename F, typename Outputs, typename Inputs, typename Scratches>
inline std::shared_ptr<program> make_functor_program(
	F &&fun,
	Outputs,
	Inputs,
	Scratches,
	std::array<program_scratch_requirement, type_list_size<Scratches>::value>
		scratch_requirements
)
{
	using program_type = functor_program<
		std::decay_t<F>, Outputs, Inputs, Scratches
	>;
	return std::make_shared<program_type>(
		std::forward<F>(fun),
		std::move(scratch_requirements)
	);
}

} // namespace cpu
} // namespace xmipp4
