// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_names.hpp"
#include "operation_arity.hpp"

#include "../platform/dynamic_shared_object.h"
#include "../span.hpp"

#include <cstddef>
#include <ostream>
#include <string>

namespace rexlib
{

/**
 * @brief Immutable static description of an operation type.
 *
 * Holds everything that is the same for every instance of a given
 * operation: the component it belongs to, its name, and the name of each of
 * its operands. The operand names also fix the arity, so an operation
 * cannot report an arity that disagrees with the operands it declares.
 *
 * Nothing here is owned. Every string and every name list must have static
 * storage duration, which the declaration macros guarantee.
 *
 * @see basic_operation
 */
class operation_descriptor
{
public:
	/**
	 * @brief Construct a descriptor.
	 *
	 * @param component Name of the component owning the operation, such as
	 * "rex.ops". Used to qualify the operation in diagnostics and to
	 * keep names from different components apart.
	 * @param name Name of the operation, such as "add".
	 * @param output_operand_names Names of the output operands, in
	 * signature order.
	 * @param input_operand_names Names of the input operands, in signature
	 * order.
	 */
	REXLIB_API
	operation_descriptor(
		const char *component,
		const char *name,
		span<const char* const> output_operand_names,
		span<const char* const> input_operand_names
	) noexcept;

	operation_descriptor(const operation_descriptor &other) = default;
	operation_descriptor(operation_descriptor &&other) noexcept = default;
	~operation_descriptor() = default;

	operation_descriptor&
	operator=(const operation_descriptor &other) = default;
	operation_descriptor&
	operator=(operation_descriptor &&other) noexcept = default;

	/**
	 * @brief Get the component owning the operation.
	 *
	 * @return const char* The component name.
	 */
	REXLIB_API
	const char* get_component() const noexcept;

	/**
	 * @brief Get the name of the operation.
	 *
	 * @return const char* The name.
	 */
	REXLIB_API
	const char* get_name() const noexcept;

	/**
	 * @brief Get the operand counts of the operation.
	 *
	 * @return operation_arity The arity.
	 */
	REXLIB_API
	operation_arity get_arity() const noexcept;

	/**
	 * @brief Get the names of the output operands.
	 *
	 * @return span<const char* const> The names, in signature order.
	 */
	REXLIB_API
	span<const char* const> get_output_operand_names() const noexcept;

	/**
	 * @brief Get the names of the input operands.
	 *
	 * @return span<const char* const> The names, in signature order.
	 */
	REXLIB_API
	span<const char* const> get_input_operand_names() const noexcept;

	/**
	 * @brief Get the name of one operand.
	 *
	 * @param index Index of the operand.
	 * @param output_operand Whether the operand is an output.
	 * @return const char* The name, or null when the index is out of
	 * range. Building a diagnostic must not be able to fail, so callers are
	 * expected to tolerate a missing name rather than assert on it.
	 */
	REXLIB_API
	const char* get_operand_name(
		std::size_t index,
		bool output_operand
	) const noexcept;

	/**
	 * @brief Write the qualified name of an operation.
	 *
	 * The component and the name are joined with a dot, as in
	 * "rex.ops.add".
	 *
	 * @param os The stream where the descriptor is written.
	 * @param descriptor The descriptor to be written.
	 * @return std::ostream& The stream.
	 */
	friend std::ostream& operator<<(
		std::ostream &os,
		const operation_descriptor &descriptor
	)
	{
		return os << descriptor.get_component() << '.' << descriptor.get_name();
	}

private:
	const char *m_component;
	const char *m_name;
	span<const char* const> m_output_operand_names;
	span<const char* const> m_input_operand_names;
};

/**
 * @brief Describe an operand for a diagnostic.
 *
 * Yields the operand's quoted name when it has one and its index otherwise,
 * so that building a message never depends on a name being present.
 *
 * @param descriptor The operation description.
 * @param index Index of the operand.
 * @param output_operand Whether the operand is an output.
 * @return std::string The description.
 */
REXLIB_API
std::string describe_operand(
	const operation_descriptor &descriptor,
	std::size_t index,
	bool output_operand
);

/**
 * @brief Build a descriptor from compile time operand name lists.
 *
 * @tparam OutputCount Number of output operands.
 * @tparam InputCount Number of input operands.
 * @param component Name of the component owning the operation.
 * @param name Name of the operation.
 * @param outputs Names of the output operands.
 * @param inputs Names of the input operands.
 * @return operation_descriptor The descriptor.
 */
template <std::size_t OutputCount, std::size_t InputCount>
operation_descriptor make_operation_descriptor(
	const char *component,
	const char *name,
	const operand_names<OutputCount> &outputs,
	const operand_names<InputCount> &inputs
) noexcept
{
	return operation_descriptor(
		component,
		name,
		outputs.get(),
		inputs.get()
	);
}

} // namespace rexlib
