// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/service_catalog.hpp>

#include <xmipp4/core/plugin.hpp>

#include "plugin_loader.hpp"

#include <vector>
#include <cstdlib>
#include <unordered_map>

namespace rex
{

class service_catalog::implementation
{
public:
	explicit implementation(bool register_builtin_backends)
		: m_register_builtin_backends(register_builtin_backends)
	{
	}

	~implementation() = default;

	std::shared_ptr<service_manager>
	get_service_manager(std::type_index key)
	{
		const auto ite = m_interfaces.find(key);
		if(ite != m_interfaces.end())
		{
			return ite->second;
		}

		return nullptr;
	}

	void create_service_manager(
		std::type_index key,
		std::shared_ptr<service_manager> value
	)
	{
		if (m_register_builtin_backends)
		{
			value->register_builtin_backends();
		}

		m_interfaces.emplace(key, std::move(value));
	}

private:
	using catalog_type = 
		std::unordered_map<std::type_index, std::shared_ptr<service_manager>>;

	bool m_register_builtin_backends;
	catalog_type m_interfaces;
};

service_catalog::service_catalog(bool register_builtin_backends)
	: m_implementation(
		std::make_unique<implementation>(register_builtin_backends)
	)
{
}

service_catalog::service_catalog(service_catalog&& other) noexcept = default;

service_catalog::~service_catalog() = default;

service_catalog& 
service_catalog::operator=(service_catalog&& other) noexcept = default;

std::shared_ptr<service_manager>
service_catalog::get_service_manager(std::type_index key)
{
	return m_implementation->get_service_manager(key);
}
		
void service_catalog::create_service_manager(
	std::type_index key,
	std::shared_ptr<service_manager> value
)
{
	m_implementation->create_service_manager(key, std::move(value));
}

} // namespace rex
