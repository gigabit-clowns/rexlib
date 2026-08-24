// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/hardware/device_properties.hpp>

namespace rexlib
{

class device_properties::implementation
{
public:
	implementation() noexcept
		: m_type(device_type::unknown)
		, m_total_memory_bytes(0)
		, m_optimal_data_alignment(0)
	{
	}

	void set_type(device_type type) noexcept
	{
		m_type = type;
	}

	device_type get_type() const noexcept
	{
		return m_type;
	}

	void set_name(const std::string &name)
	{
		m_name = name;
	}

	const std::string& get_name() const noexcept
	{
		return m_name;
	}

	void set_physical_location(const std::string &location)
	{
		m_physical_location = location;
	}

	const std::string& get_physical_location() const noexcept
	{
		return m_physical_location;
	}

	void set_total_memory_bytes(std::size_t bytes) noexcept
	{
		m_total_memory_bytes = bytes;
	}

	std::size_t get_total_memory_bytes() const noexcept
	{
		return m_total_memory_bytes;
	}

	void set_optimal_data_alignment(std::size_t alignment) noexcept
	{
		m_optimal_data_alignment = alignment;
	}

	std::size_t get_optimal_data_alignment() const noexcept
	{
		return m_optimal_data_alignment;
	}

private:
	device_type m_type;
	std::string m_name;
	std::string m_physical_location;
	std::size_t m_total_memory_bytes;
	std::size_t m_optimal_data_alignment;
};


device_properties::device_properties() noexcept = default;

device_properties::device_properties(const device_properties &other)
{
	*this = other;
}

device_properties::device_properties(
	device_properties&& other
) noexcept = default;

device_properties::~device_properties() = default;

device_properties& 
device_properties::operator=(const device_properties &other)
{
	if (other.m_implementation)
	{
		m_implementation = 
			std::make_unique<implementation>(*other.m_implementation);
	}

	return *this;
}

device_properties& 
device_properties::operator=(device_properties&& other) noexcept = default;

void device_properties::set_type(device_type type)
{
	create_implementation_if_null().set_type(type);
}

device_type device_properties::get_type() const noexcept
{
	return get_implementation().get_type();
}

void device_properties::set_name(const std::string &name)
{
	create_implementation_if_null().set_name(name);
}

const std::string& device_properties::get_name() const noexcept
{
	return get_implementation().get_name();
}

void device_properties::set_physical_location(const std::string &location)
{
	create_implementation_if_null().set_physical_location(location);
}

const std::string& device_properties::get_physical_location() const noexcept
{
	return get_implementation().get_physical_location();
}

void device_properties::set_total_memory_bytes(std::size_t bytes)
{
	create_implementation_if_null().set_total_memory_bytes(bytes);
}

std::size_t device_properties::get_total_memory_bytes() const noexcept
{
	return get_implementation().get_total_memory_bytes();
}

void device_properties::set_optimal_data_alignment(
	std::size_t alignment
)
{
	create_implementation_if_null().set_optimal_data_alignment(alignment);
}

std::size_t device_properties::get_optimal_data_alignment() const noexcept
{
	return get_implementation().get_optimal_data_alignment();
}

device_properties::implementation& 
device_properties::create_implementation_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}

	return *m_implementation;
}

const device_properties::implementation& 
device_properties::get_implementation() const noexcept
{
	static const implementation empty;
	return m_implementation ? *m_implementation : empty;
}

} // namespace rexlib
