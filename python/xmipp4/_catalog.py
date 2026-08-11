# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
from typing import Optional
from ._core_binding import ServiceCatalog

__default_catalog: Optional[ServiceCatalog] = None

def get_default_catalog() -> ServiceCatalog:
	"""
	Get the process-wide default service catalog.

	xmipp4::service_catalog is not a singleton in C++ (its uniqueness is
	scoped to the owning instance). This function provides a lazily
	constructed, process-wide instance for callers that don't need to
	manage their own catalog.

	Returns:
		ServiceCatalog: The default service catalog. The same instance is
		returned on every call.
	"""
	global __default_catalog
	if __default_catalog is None:
		__default_catalog = ServiceCatalog()
	return __default_catalog
