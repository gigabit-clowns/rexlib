// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/ndarray/const_array.hpp>

#include <rexlib/core/ndarray/array.hpp>

#include "array_implementation.hpp"

namespace rexlib 
{

const_array::const_array() = default;
const_array::const_array(const_array&& other) noexcept = default;
const_array::~const_array() = default;
const_array& const_array::operator=(const_array&& other) noexcept = default;

const_array::const_array(
	std::shared_ptr<const array_implementation> implementation
) noexcept
	: m_implementation(std::move(implementation))
{
}

const array_descriptor& const_array::get_descriptor() const noexcept
{
	static array_descriptor empty_descriptor;
	return
		m_implementation ? 
		m_implementation->get_descriptor() : 
		empty_descriptor;
}

const buffer* const_array::get_storage() const noexcept
{
	return
		m_implementation ? 
		m_implementation->get_storage() : 
		nullptr;
}

std::shared_ptr<const buffer> 
const_array::share_storage() const noexcept
{
	return
		m_implementation ? 
		m_implementation->share_storage() : 
		nullptr;
}

const_array const_array::share() const noexcept
{
	return const_array(m_implementation);
}

const array_implementation*
const_array::get_implementation() const noexcept
{
	return m_implementation.get();
}

} // namespace rexlib
