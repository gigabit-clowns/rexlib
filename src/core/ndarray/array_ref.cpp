// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/ndarray/array_ref.hpp>

#include <rexlib/core/ndarray/array.hpp>

#include "array_implementation.hpp"

namespace rexlib
{

array_ref::array_ref() noexcept
	: array_ref(nullptr)
{
}

array_ref::array_ref(const array_implementation *implementation) noexcept
	: m_implementation(implementation)
{
}

array_ref::array_ref(array &other) noexcept
	: array_ref(other.get_implementation())
{
}

const array_descriptor& array_ref::get_descriptor() const noexcept
{
	static const array_descriptor empty_descriptor;
	return
		m_implementation ?
		m_implementation->get_descriptor() :
		empty_descriptor;
}

buffer* array_ref::get_storage() noexcept
{
	return
		m_implementation ?
		m_implementation->get_storage() :
		nullptr;
}

const buffer* array_ref::get_storage() const noexcept
{
	return
		m_implementation ?
		m_implementation->get_storage() :
		nullptr;
}

std::shared_ptr<buffer> array_ref::share_storage() noexcept
{
	return
		m_implementation ?
		m_implementation->share_storage() :
		nullptr;
}

std::shared_ptr<const buffer> array_ref::share_storage() const noexcept
{
	return
		m_implementation ?
		m_implementation->share_storage() :
		nullptr;
}

const array_implementation* array_ref::get_implementation() const noexcept
{
	return m_implementation;
}

} // namespace rexlib
