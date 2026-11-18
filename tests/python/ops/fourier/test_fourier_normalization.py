# SPDX-License-Identifier: GPL-3.0-only

import rexlib

def test_fourier_normalization_names_the_three_conventions():
	assert {member.name for member in rexlib.FourierNormalization} == {
		"backward", "ortho", "forward"
	}

def test_fourier_normalization_is_found_by_name():
	assert (
		rexlib.FourierNormalization["ortho"]
		== rexlib.FourierNormalization.ortho
	)
