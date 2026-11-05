# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations

from ._binding import PluginManager, ServiceCatalog
from ._plugins import discover_entry_point_plugins

__default_catalog: ServiceCatalog | None = None

def get_default_catalog() -> ServiceCatalog:
	"""
	Get the process-wide default service catalog.

	rexlib::service_catalog is not a singleton in C++ (its uniqueness is
	scoped to the owning instance). This function provides a lazily
	constructed, process-wide instance for callers that don't need to
	manage their own catalog. On construction it registers the plugins
	that installed distributions declare; see
	`rexlib.discover_entry_point_plugins`.

	Returns:
		ServiceCatalog: The default service catalog. The same instance is
		returned on every call.
	"""
	global __default_catalog  # noqa: PLW0603 -- lazily-initialized singleton
	if __default_catalog is None:
		catalog = ServiceCatalog()
		manager = PluginManager()
		discover_entry_point_plugins(manager)
		catalog.register_plugins(manager)
		__default_catalog = catalog
	return __default_catalog
