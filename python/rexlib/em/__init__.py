# SPDX-License-Identifier: GPL-3.0-only

"""Electron microscopy file formats.

One module per area of `rexlib::em`, named after the directory that area
occupies in rexlib, the way every module here is named. Each carries the
vocabulary of its own area, so `read` and `write` mean one thing apiece
and the areas arriving after images rename nothing.
"""

from __future__ import annotations

from . import image as image

__all__ = ["image"]
