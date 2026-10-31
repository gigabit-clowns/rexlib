# SPDX-License-Identifier: GPL-3.0-only

"""Electron microscopy.

The operations of `rexlib::em`, here rather than in a submodule of
their own, as the operations of `rexlib` are at the top of `rexlib`.
Each takes its execution context from the active one unless given one.
"""

from __future__ import annotations

from ._functional import (
	standardize_background as standardize_background,
	standardize_background_2d as standardize_background_2d,
	standardize_background_3d as standardize_background_3d,
)

__all__ = [
	"standardize_background",
	"standardize_background_2d",
	"standardize_background_3d",
]
