// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "service.hpp"
#include "platform/dynamic_shared_object.h"

#include <memory>
#include <typeindex>
#include <type_traits>

namespace rexlib
{

/**
 * @brief Hold one instance of each kind of service.
 *
 * @see service
 */
class service_catalog
{
public:
	/**
	 * @brief Construct a new interface catalog object.
	 *
	 * @param register_builtin_backends Whether the catalog registers the
	 * plugins built into the library at itself, as a loaded plugin is
	 * registered.
	 */
	REXLIB_API
	explicit service_catalog(bool register_builtin_backends = true);
	service_catalog(const service_catalog &other) = delete;
	REXLIB_API service_catalog(service_catalog &&other) noexcept;
	REXLIB_API ~service_catalog();

	service_catalog &operator=(const service_catalog &other) = delete;
	REXLIB_API
	service_catalog &operator=(service_catalog &&other) noexcept;

	/**
	 * @brief Retrieve the unique instance of a concrete service.
	 *
	 * Always returns the same instance of @p T held by this catalog,
	 * constructing it on first access, so within a given catalog the returned
	 * pointer behaves like a singleton. These are not true singletons:
	 * the uniqueness is scoped to the owning service_catalog rather than
	 * to the whole process, which keeps service instances out of
	 * global state.
	 *
	 * @tparam T Concrete type derived from service to retrieve.
	 * @return Shared pointer to the catalog-owned instance of @p T. Never null.
	 *
	 * @see service
	 */
	template <typename T>
	std::shared_ptr<T> get_service();

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;

	REXLIB_API
	std::shared_ptr<service> get_service(std::type_index key);

	REXLIB_API
	void create_service(
		std::type_index key,
		std::shared_ptr<service> instance
	);
};

} // namespace rexlib

#include "service_catalog.inl"
