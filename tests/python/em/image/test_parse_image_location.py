# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

ImageLocation = rexlib.em.image.ImageLocation
parse_image_location = rexlib.em.image.parse_image_location

def test_bare_path_addresses_the_whole_file():
	location = parse_image_location('stack.mrc')
	assert location == ImageLocation('stack.mrc')

def test_index_is_read_one_based():
	location = parse_image_location('3@stack.mrc')
	assert location == ImageLocation('stack.mrc', 2)

@pytest.mark.parametrize(
	"text",
	[
		pytest.param('stack.mrc', id="Whole file"),
		pytest.param('3@stack.mrc', id="Position in a stack"),
	]
)
def test_inverse_of_str(text):
	assert str(parse_image_location(text)) == text

@pytest.mark.parametrize(
	"text",
	[
		pytest.param('0@stack.mrc', id="Zero is not a valid index"),
		pytest.param('@stack.mrc', id="Missing index"),
		pytest.param('x@stack.mrc', id="Index is not a number"),
	]
)
def test_returns_none_when_it_cannot_parse(text):
	assert parse_image_location(text) is None

def test_a_path_is_never_parsed_on_its_way_into_a_location():
	assert ImageLocation('3@stack.mrc').path == '3@stack.mrc'
