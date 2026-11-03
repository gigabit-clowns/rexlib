# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_ctf_optics_keeps_its_constants():
	optics = rexlib.em.CtfOptics(300, 2.7, 0.1)

	assert optics.voltage == 300
	assert optics.spherical_aberration == 2.7
	assert optics.amplitude_contrast == 0.1
	assert optics.wavelength == pytest.approx(0.019687615309, rel=1e-9)
	assert "CtfOptics" in repr(optics)

@pytest.mark.parametrize(
	"voltage, spherical_aberration, amplitude_contrast",
	[(0, 2.7, 0.1), (300, float("inf"), 0.1), (300, 2.7, 1.0)]
)
def test_ctf_optics_rejects_invalid_values(
	voltage, spherical_aberration, amplitude_contrast
):
	with pytest.raises(ValueError):
		rexlib.em.CtfOptics(voltage, spherical_aberration, amplitude_contrast)
