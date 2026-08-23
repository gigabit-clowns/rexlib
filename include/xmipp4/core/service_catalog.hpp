// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "service_manager.hpp"
#include "platform/dynamic_shared_object.h"

#include <memory>
#include <typeindex>
#include <type_traits>

namespace xmipp4
{

/**
 * @brief Stores a single instance of each type service_manager.
 *
 * The interface catalog allows to centralize interface managers.
 *
 * @see service_manager
 */
class service_catalog
{
public:
	/**
	 * @brief Construct a new interface catalog object.
	 *
	 * @param register_builtin_backends This parameters controls whether the
	 * interface catalog should load the backends bundled with the
	 * core.
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
	 * @brief Retrieve the unique instance of a concrete service_manager.
	 *
	 * Always returns the same instance of @p T held by this catalog, 
	 * constructing it on first access, so within a given catalog the returned
	 * pointer behaves like a singleton. These are not true singletons:
	 * the uniqueness is scoped to the owning service_catalog rather than
	 * to the whole process, which keeps service_manager instances out of
	 * global state.
	 *
	 * @tparam T Concrete type derived from service_manager to retrieve.
	 * @return Shared pointer to the catalog-owned instance of @p T. Never null.
	 *
	 * @see service_manager
	 */
	template <typename T>
	std::shared_ptr<T> get_service_manager();

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;

	REXLIB_API
	std::shared_ptr<service_manager> get_service_manager(std::type_index key);

	REXLIB_API
	void create_service_manager(
		std::type_index key,
		std::shared_ptr<service_manager> manager
	);
};

} // namespace xmipp4

#include "service_catalog.inl"
