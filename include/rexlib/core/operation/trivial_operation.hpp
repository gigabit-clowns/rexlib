// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/basic_operation.hpp>
#include <rexlib/core/operation/operation_signature.hpp>
#include <rexlib/core/operation/shared_rule_signature.hpp>

namespace rexlib
{

/**
 * @brief CRTP base for operations that are trivially declared.
 *
 * Returns the signature shared by every operation following the rules
 * named by @p Traits, so that the body of the derived class is usually
 * empty.
 *
 * "Trivial" refers to the declaration and not to the C++ notion of a
 * trivial type: every operation has a virtual destructor, and the operation
 * may still carry values its signature does not read. What is trivial is
 * that nothing about its dispatch has to be written by hand.
 *
 * @tparam Derived The concrete operation type.
 * @tparam Traits The static description, as basic_operation requires. Its
 * rules must read nothing from the operation instance.
 *
 * @see basic_operation
 * @see shared_rule_signature
 */
template <typename Derived, typename Traits>
class trivial_operation
	: public basic_operation<Derived, Traits>
{
public:
	using typename basic_operation<Derived, Traits>::shape_rule_type;
	using typename basic_operation<Derived, Traits>::type_rule_type;

	const operation_signature& get_signature() const noexcept override;
};

} // namespace rexlib

#include "trivial_operation.inl"
