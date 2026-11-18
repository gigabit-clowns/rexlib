# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_nearest_is_a_box_of_half_width_one_half():
	window = rexlib.em.GriddingWindow.nearest()

	assert window.profile == rexlib.em.GriddingProfile.box
	assert window.half_width == 0.5
	assert window.shape == 0.0

def test_linear_is_a_triangle_of_half_width_one():
	window = rexlib.em.GriddingWindow.linear()

	assert window.profile == rexlib.em.GriddingProfile.triangle
	assert window.half_width == 1.0

def test_kaiser_bessel_keeps_its_half_width_and_shape():
	window = rexlib.em.GriddingWindow.kaiser_bessel(half_width=2, shape=9.5)

	assert window.profile == rexlib.em.GriddingProfile.kaiser_bessel
	assert window.half_width == 2.0
	assert window.shape == 9.5
	assert "kaiser_bessel" in repr(window)

@pytest.mark.parametrize("half_width, shape", [(0, 1), (2, -1)])
def test_kaiser_bessel_rejects_invalid_values(half_width, shape):
	with pytest.raises(ValueError):
		rexlib.em.GriddingWindow.kaiser_bessel(half_width, shape)
