# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_returns_read_format_manager(__setup_service_catalog):
	assert isinstance(
		rexlib.em.image.get_image_read_format_manager(__setup_service_catalog),
		rexlib.em.image.ImageReadFormatManager
	)

def test_returns_write_format_manager(__setup_service_catalog):
	assert isinstance(
		rexlib.em.image.get_image_write_format_manager(__setup_service_catalog),
		rexlib.em.image.ImageWriteFormatManager
	)

def test_always_returns_same_read_format_manager(__setup_service_catalog):
	first = rexlib.em.image.get_image_read_format_manager(
		__setup_service_catalog
	)
	second = rexlib.em.image.get_image_read_format_manager(
		__setup_service_catalog
	)
	assert first is second

def test_always_returns_same_write_format_manager(__setup_service_catalog):
	first = rexlib.em.image.get_image_write_format_manager(
		__setup_service_catalog
	)
	second = rexlib.em.image.get_image_write_format_manager(
		__setup_service_catalog
	)
	assert first is second

def test_reading_and_writing_are_separate_services(__setup_service_catalog):
	read = rexlib.em.image.get_image_read_format_manager(
		__setup_service_catalog
	)
	write = rexlib.em.image.get_image_write_format_manager(
		__setup_service_catalog
	)
	assert read is not write

def test_reachable_from_the_default_catalog():
	catalog = rexlib.get_default_catalog()
	assert rexlib.em.image.get_image_read_format_manager(catalog) is not None

@pytest.fixture
def __setup_service_catalog():
	return rexlib.ServiceCatalog()
