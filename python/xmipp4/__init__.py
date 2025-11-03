# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
from .load_core import load_core

# Heuristically load the core library before importing the binding.
__core_lib = load_core()

from ._core_binding import (
	__doc__, __version__, 
	communication, hardware,
	ServiceCatalog,
	PluginManager, Plugin, get_plugin_directory, get_default_plugin_directory,
	Version
)

__all__ = ["__doc__", "__version__", "hardware", "Version"]
