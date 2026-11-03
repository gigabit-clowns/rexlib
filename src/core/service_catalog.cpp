// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/service_catalog.hpp>

#include "builtin_plugin_registry.hpp"

#include <unordered_map>

namespace rexlib
{

class service_catalog::implementation
{
public:
	implementation() = default;

	implementation(const implementation &other) = delete;
	implementation(implementation &&other) = delete;
	~implementation() = default;

	implementation& operator=(const implementation &other) = delete;
	implementation& operator=(implementation &&other) = delete;

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
		m_interfaces.emplace(key, std::move(value));
	}

private:
	using catalog_type =
		std::unordered_map<std::type_index, std::shared_ptr<service_manager>>;

	catalog_type m_interfaces;
};

service_catalog::service_catalog(bool register_builtin_backends)
	: m_implementation(std::make_unique<implementation>())
{
	if (register_builtin_backends)
	{
		get_builtin_plugin_registry().register_all(*this);
	}
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

} // namespace rexlib
