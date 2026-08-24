// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/device_manager.hpp>

#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <backends/cpu/hardware/device_backend.hpp>

#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace rexlib
{

class device_manager::implementation
{
public:
	bool register_backend(std::unique_ptr<device_backend> backend)
	{
		if (!backend)
		{
			return false;
		}

		auto name = backend->get_name();
		bool inserted;
		std::tie(std::ignore, inserted) = m_backends.emplace(
			std::move(name),
			std::move(backend)
		);

		return inserted;
	}

	void enumerate_backends(std::vector<std::string> &names) const
	{
		names.clear();
		names.reserve(m_backends.size());
		for(const auto &item : m_backends)
		{
			names.emplace_back(item.first);
		}
	}

	device_backend* get_backend(const std::string &name) const
	{
		const auto ite = m_backends.find(name);
		if (ite == m_backends.end())
		{
			return nullptr;
		}

		return ite->second.get();
	}

	void enumerate_devices(std::vector<device_index> &indices) const
	{
		REXLIB_ASSERT( indices.empty() );

		std::vector<std::size_t> ids;
		for(const auto &item : m_backends)
		{
			const auto *backend = item.second.get();
			REXLIB_ASSERT(backend);
			backend->enumerate_devices(ids);
			for(const auto &id : ids)
			{
				indices.emplace_back(item.first, id);
			}
		}
	}

private:
	std::unordered_map<std::string, std::unique_ptr<device_backend>> m_backends;
};

device_manager::device_manager() = default;

device_manager::~device_manager() = default;

void device_manager::register_builtin_backends()
{
	cpu::device_backend::register_at(*this);
}

bool device_manager::register_backend(std::unique_ptr<device_backend> backend)
{
	create_implementation_if_null();
	return m_implementation->register_backend(std::move(backend));
}

void device_manager::enumerate_backends(
	std::vector<std::string> &names
) const
{
	names.clear();
	if (m_implementation)
	{
		m_implementation->enumerate_backends(names);
	}
}

device_backend* device_manager::get_backend(const std::string &name) const
{
	if (!m_implementation)
	{
		return nullptr;
	}

	return m_implementation->get_backend(name);
}

void device_manager::enumerate_devices(std::vector<device_index> &indices) const
{
	indices.clear();
	if (m_implementation)
	{
		m_implementation->enumerate_devices(indices);
	}
}

bool device_manager::get_device_properties(
	const device_index &index, 
	device_properties &desc 
) const
{
	const auto *backend = get_backend(index.get_backend_name());
	if (!backend)
	{
		return false;
	}   

	return backend->get_device_properties(index.get_device_id(), desc);
}

std::shared_ptr<device_session>
device_manager::create_device_session(const device_index& index) const
{
	const auto *backend = get_backend(index.get_backend_name());
	if (!backend)
	{
		throw std::invalid_argument("Requested backend does not exist");
	}

	const auto id = index.get_device_id();
	auto dev = backend->create_device(id);

	device_properties properties;
	if (!backend->get_device_properties(id, properties))
	{
		throw std::invalid_argument("Requested device does not exist");
	}

	return std::make_shared<device_session>(
		std::move(dev),
		std::move(properties)
	);
}

void device_manager::create_implementation_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}
}

} // namespace rexlib
