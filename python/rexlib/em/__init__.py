# SPDX-License-Identifier: GPL-3.0-only

"""Electron microscopy.

The operations of `rexlib::em`, here rather than in a submodule of
their own, as the operations of `rexlib` are at the top of `rexlib`.
Each takes its execution context from the active one unless given one.
The types their parameters are made of are here too.
"""

from __future__ import annotations

from .._binding.em.ops.parameters import (
	CtfOptics as CtfOptics,
	GriddingProfile as GriddingProfile,
	GriddingWindow as GriddingWindow,
)
from ._functional import (
	insert_ctf_slices as insert_ctf_slices,
	insert_slices as insert_slices,
	insert_weighted_slices as insert_weighted_slices,
	standardize_background as standardize_background,
	standardize_background_2d as standardize_background_2d,
	standardize_background_3d as standardize_background_3d,
)

__all__ = [
	"CtfOptics",
	"GriddingProfile",
	"GriddingWindow",
	"insert_ctf_slices",
	"insert_slices",
	"insert_weighted_slices",
	"standardize_background",
	"standardize_background_2d",
	"standardize_background_3d",
]
