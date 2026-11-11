# SPDX-License-Identifier: GPL-3.0-only

import pathlib

import rexlib

def test_the_package_ships_the_type_stubs_of_the_extension():
	stubs = pathlib.Path(rexlib.__file__).parent / '_binding'
	assert (stubs / '__init__.pyi').is_file()

def test_every_stub_says_it_is_generated():
	stubs = pathlib.Path(rexlib.__file__).parent / '_binding'
	for stub in stubs.rglob('*.pyi'):
		assert stub.read_text(encoding='utf-8').startswith('# Generated'), stub
