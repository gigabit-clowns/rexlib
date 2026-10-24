// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/operand_descriptor.hpp>

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <boost/functional/hash.hpp>

namespace rexlib
{

namespace
{

const memory_resource* get_memory_resource(const buffer *storage)
{
	if (!storage)
	{
		return nullptr;
	}

	return &storage->get_memory_resource();
}

} // anonymous namespace

operand_descriptor::operand_descriptor() noexcept
	: m_descriptor()
	, m_memory_region(nullptr)
{
}

operand_descriptor::operand_descriptor(
	const array_descriptor &descriptor,
	const memory_resource* resource
) noexcept
	: m_descriptor(descriptor)
	, m_memory_region(resource)
{
}

operand_descriptor::operand_descriptor(
	array_descriptor &&descriptor,
	const memory_resource* resource
) noexcept
	: m_descriptor(std::move(descriptor))
	, m_memory_region(resource)
{
}

operand_descriptor::operand_descriptor(
	const strided_layout& layout,
	numerical_type data_type,
	const memory_resource* resource
) noexcept
	: m_descriptor(layout, data_type)
	, m_memory_region(resource)
{
}

operand_descriptor::operand_descriptor(
	strided_layout&& layout,
	numerical_type data_type,
	const memory_resource* resource
) noexcept
	: m_descriptor(std::move(layout), data_type)
	, m_memory_region(resource)
{
}

operand_descriptor::operand_descriptor(
	const operand_descriptor &other
) = default;
operand_descriptor::operand_descriptor(
	operand_descriptor &&other
) noexcept = default;
operand_descriptor::~operand_descriptor() = default;

operand_descriptor&
operand_descriptor::operator=(const operand_descriptor &other) = default;
operand_descriptor&
operand_descriptor::operator=(operand_descriptor &&other) noexcept = default;

std::size_t operand_descriptor::hash() const noexcept
{
	auto seed = m_descriptor.hash();
	boost::hash_combine(seed, boost::hash_value(m_memory_region));
	return seed;
}

const strided_layout& operand_descriptor::get_layout() const noexcept
{
	return m_descriptor.get_layout();
}

numerical_type operand_descriptor::get_data_type() const noexcept
{
	return m_descriptor.get_data_type();
}

const array_descriptor&
operand_descriptor::get_array_descriptor() const noexcept
{
	return m_descriptor;
}

void operand_descriptor::set_array_descriptor(
	const array_descriptor &descriptor
) noexcept
{
	m_descriptor = descriptor;
}

const memory_resource*
operand_descriptor::get_memory_resource() const noexcept
{
	return m_memory_region;
}

void operand_descriptor::set_memory_resource(
	const memory_resource* resource
) noexcept
{
	m_memory_region = resource;
}

operand_descriptor
operand_descriptor::from_array(const_array_ref a) noexcept
{
	return operand_descriptor(
		a.get_descriptor(),
		rexlib::get_memory_resource(a.get_storage())
	);
}

} // namespace rexlib
