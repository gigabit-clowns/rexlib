# SPDX-License-Identifier: GPL-3.0-only

import rexlib

def test_returns_a_catalog():
	assert rexlib.get_default_catalog() is not None

def test_always_returns_the_same_catalog():
	c1 = rexlib.get_default_catalog()
	c2 = rexlib.get_default_catalog()
	assert c1 is c2
