// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/ndarray/const_array_ref.hpp>

#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array.hpp>

#include "array_implementation.hpp"

namespace rexlib
{

const_array_ref::const_array_ref() noexcept
	: const_array_ref(nullptr)
{
}

const_array_ref::const_array_ref(const array_implementation *implementation) noexcept
	: m_implementation(implementation)
{
}

const_array_ref::const_array_ref(const array &other) noexcept
	: const_array_ref(other.get_implementation())
{
}

const_array_ref::const_array_ref(const const_array &other) noexcept
	: const_array_ref(other.get_implementation())
{
}

const_array_ref::const_array_ref(array_ref other) noexcept
	: const_array_ref(other.get_implementation())
{
}

const array_descriptor& const_array_ref::get_descriptor() const noexcept
{
	static const array_descriptor empty_descriptor;
	return
		m_implementation ?
		m_implementation->get_descriptor() :
		empty_descriptor;
}

const buffer* const_array_ref::get_storage() const noexcept
{
	return
		m_implementation ?
		m_implementation->get_storage() :
		nullptr;
}

std::shared_ptr<const buffer>
const_array_ref::share_storage() const noexcept
{
	return
		m_implementation ?
		m_implementation->share_storage() :
		nullptr;
}

const array_implementation*
const_array_ref::get_implementation() const noexcept
{
	return m_implementation;
}

} // namespace rexlib
