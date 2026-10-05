# SPDX-License-Identifier: GPL-3.0-only

import os

import pytest

import rexlib

image = rexlib.em.image

def test_starts_with_no_file_declared():
	assert image.writer_provider().file_count == 0

def test_returns_a_managed_provider():
	writers = image.writer_provider()
	assert isinstance(writers, image.ManagedImageWriterProvider)
	assert isinstance(writers, image.ImageWriterProvider)

def test_counts_the_files_it_was_declared(tmp_path):
	writers = image.writer_provider()
	writers.declare(str(tmp_path / 'first.mrcs'), __setup_descriptor())
	writers.declare(str(tmp_path / 'second.mrcs'), __setup_descriptor())
	assert writers.file_count == 2

def test_arguments_are_accepted_by_name(tmp_path):
	writers = image.writer_provider()
	writers.declare(
		path=str(tmp_path / 'stack.mrcs'), descriptor=__setup_descriptor()
	)
	assert writers.file_count == 1

def test_declaring_creates_no_file(tmp_path):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, __setup_descriptor())
	assert not os.path.exists(path)

def test_a_path_declared_twice_is_refused(tmp_path):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	descriptor = __setup_descriptor()
	writers.declare(path, descriptor)
	with pytest.raises(ValueError):
		writers.declare(path, descriptor)

def test_closing_forgets_the_file(tmp_path):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, __setup_descriptor())
	writers.close(path)
	assert writers.file_count == 0

def test_a_closed_path_can_be_declared_again(tmp_path):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, __setup_descriptor())
	writers.close(path)
	writers.declare(path, __setup_descriptor())
	assert writers.file_count == 1

def test_closing_a_path_that_was_not_declared_is_refused(tmp_path):
	writers = image.writer_provider()
	with pytest.raises(IndexError):
		writers.close(str(tmp_path / 'stack.mrcs'))

def test_flushing_with_nothing_written_creates_no_file(tmp_path):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, __setup_descriptor())
	writers.flush()
	assert not os.path.exists(path)

def test_accepts_an_explicit_manager(tmp_path):
	catalog = rexlib.ServiceCatalog()
	writers = image.writer_provider(
		image.get_image_write_format_manager(catalog)
	)
	writers.declare(str(tmp_path / 'stack.mrcs'), __setup_descriptor())
	assert writers.file_count == 1

def test_assembled_by_hand(tmp_path):
	catalog = rexlib.ServiceCatalog()
	writers = image.ManagedImageWriterProvider(
		image.get_image_write_format_manager(catalog)
	)
	writers.declare(str(tmp_path / 'stack.mrcs'), __setup_descriptor())
	assert writers.file_count == 1

def __setup_descriptor():
	return image.ImageDescriptor((3, 4, 6), 2, rexlib.NumericalType.float32)
