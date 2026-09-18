# SPDX-License-Identifier: GPL-3.0-only

import pickle

import pytest

import rexlib

ImageLocation = rexlib.em.image.ImageLocation

def test_default_addresses_nothing():
	location = ImageLocation()
	assert location.path == ''
	assert location.position_in_stack == ImageLocation.no_position

def test_path_alone_addresses_the_whole_file():
	location = ImageLocation('stack.mrc')
	assert location.path == 'stack.mrc'
	assert location.position_in_stack == ImageLocation.no_position

def test_position_is_kept_as_given():
	location = ImageLocation('stack.mrc', 2)
	assert location.path == 'stack.mrc'
	assert location.position_in_stack == 2

def test_equal_locations_compare_equal():
	first = ImageLocation('stack.mrc', 2)
	second = ImageLocation('stack.mrc', 2)
	assert first == second

@pytest.mark.parametrize(
	"other",
	[
		pytest.param(ImageLocation('other.mrc', 2), id="Different path"),
		pytest.param(ImageLocation('stack.mrc', 3), id="Different position"),
		pytest.param(ImageLocation('stack.mrc'), id="Whole file"),
	]
)
def test_different_locations_compare_not_equal(other):
	assert ImageLocation('stack.mrc', 2) != other

def test_orders_by_path_then_position():
	assert ImageLocation('a.mrc', 9) < ImageLocation('b.mrc', 0)
	assert ImageLocation('a.mrc', 0) < ImageLocation('a.mrc', 1)

def test_equal_locations_hash_equal():
	first = ImageLocation('stack.mrc', 2)
	second = ImageLocation('stack.mrc', 2)
	assert hash(first) == hash(second)

def test_usable_as_a_dictionary_key():
	table = {ImageLocation('stack.mrc', 2): 'value'}
	assert table[ImageLocation('stack.mrc', 2)] == 'value'

def test_whole_file_is_written_as_a_bare_path():
	assert str(ImageLocation('stack.mrc')) == 'stack.mrc'

def test_position_is_written_one_based():
	assert str(ImageLocation('stack.mrc', 2)) == '3@stack.mrc'

def test_repr_names_the_type():
	assert repr(ImageLocation('stack.mrc', 2)).startswith('ImageLocation(')

def test_pickle():
	location = ImageLocation('stack.mrc', 2)
	assert pickle.loads(pickle.dumps(location)) == location

@pytest.mark.parametrize(
	"text, expected",
	[
		pytest.param('stack.mrc', ImageLocation('stack.mrc'), id="Whole file"),
		pytest.param('3@stack.mrc', ImageLocation('stack.mrc', 2), id="One based index"),
	]
)
def test_from_string_reads_a_location(text, expected):
	assert ImageLocation.from_string(text) == expected

@pytest.mark.parametrize(
	"text",
	[
		pytest.param('stack.mrc', id="Whole file"),
		pytest.param('3@stack.mrc', id="Position in a stack"),
	]
)
def test_from_string_is_the_inverse_of_str(text):
	assert str(ImageLocation.from_string(text)) == text

@pytest.mark.parametrize(
	"text",
	[
		pytest.param('', id="Empty string"),
		pytest.param('0@stack.mrc', id="Zero is not a valid index"),
		pytest.param('@stack.mrc', id="Empty index"),
		pytest.param('x@stack.mrc', id="Index is not a number"),
		pytest.param('3@', id="Empty path"),
	]
)
def test_from_string_raises_value_error_with_invalid_string(text):
	with pytest.raises(ValueError):
		ImageLocation.from_string(text)

def test_the_constructor_never_parses():
	assert ImageLocation('3@stack.mrc').path == '3@stack.mrc'
