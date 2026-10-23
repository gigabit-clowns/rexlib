// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/contract/operand_contract.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/operation/operation_id.hpp>
#include <rexlib/core/operation/operation_arity.hpp>

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <ostream>
#include <string>

namespace rexlib
{

class operation_id;

/**
 * @brief Abstract class that describes an operation.
 *
 * Subclasses of it may implement the specific behavior for each of the
 * operations.
 */
class REXLIB_API operation
{
public:
	operation() noexcept;
	operation(const operation &other) = delete;
	operation(operation &&other) = delete;
    virtual ~operation();

	operation& operator=(const operation &other) = delete;
	operation& operator=(operation &&other) = delete;

	/**
	 * @brief Get an identifier that uniquely represents this operation.
	 *
	 * The identifier is what binds an operation to the program builders
	 * implementing it, so a subclass that behaves as its base must report
	 * the base's identifier rather than one of its own. The default
	 * implementation derives it from the dynamic type and therefore does
	 * not; override it, as @ref basic_operation does, whenever an
	 * operation may be derived from.
	 *
	 * @return operation_id The operation id.
	 */
	virtual
	operation_id get_id() const noexcept;

	/**
	 * @brief Get the static description of this operation.
	 *
	 * Holds the operation's name and the name of each of its operands, so
	 * that whatever rejects a launch configuration can say which operand it
	 * objected to. @ref basic_operation supplies it from the operation's
	 * declaration.
	 *
	 * @return const operation_descriptor& The description.
	 */
	virtual
	const operation_descriptor& get_descriptor() const noexcept = 0;

	/**
	 * @brief Get a human readable identifier of the operation.
	 *
	 * This representation should not encode the parameters of the operation,
	 * i.e., it should be the same for all instances of a given operation class.
	 *
	 * @return std::string The human readable representation.
	 */
	virtual
	std::string get_name() const = 0;

	/**
	 * @brief Get the input and output count of the operation.
	 *
	 * @return operation_arity The input and output count.
	 */
	virtual
	operation_arity get_arity() const noexcept = 0;

	/**
	 * @brief Get the contract deciding the shape and data type of the
	 * operands of this operation.
	 *
	 * @return const operand_contract& The contract.
	 */
	virtual
	const operand_contract& get_contract() const noexcept = 0;
};

} // namespace rexlib
