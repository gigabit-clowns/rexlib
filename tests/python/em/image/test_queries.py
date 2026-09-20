# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_reports_the_extents_it_was_written_with(__written_file):
	assert image.query_extents(__written_file) == (4, 6)

def test_extents_are_a_tuple(__written_file):
	assert isinstance(image.query_extents(__written_file), tuple)

def test_core_extents_are_a_tuple(__written_file):
	assert isinstance(image.query_core_extents(__written_file), tuple)

def test_core_extents_are_the_trailing_extents(__written_file):
	# The axes a file stacks along are left out, so whatever the rank, the
	# core is the tail of the whole.
	extents = image.query_extents(__written_file)
	core = image.query_core_extents(__written_file)
	assert len(core) <= len(extents)
	assert extents[len(extents) - len(core):] == core

def test_what_it_reports_sizes_a_destination(__written_file, __setup_context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		(3, *image.query_core_extents(__written_file)),
		rexlib.NumericalType.float32
	)
	destination = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, __setup_context
	)
	assert destination.shape[1:] == image.query_core_extents(__written_file)

def test_accepts_an_explicit_manager(__written_file):
	catalog = rexlib.ServiceCatalog()
	manager = image.get_image_read_format_manager(catalog)
	assert image.query_extents(__written_file, manager) == (4, 6)

@pytest.mark.parametrize(
	"query",
	[
		pytest.param(image.query_extents, id="Whole file"),
		pytest.param(image.query_core_extents, id="One image"),
	]
)
def test_raises_on_a_file_no_format_recognizes(query):
	with pytest.raises(RuntimeError):
		query('/path/to/no/such/file.mrc')

@pytest.fixture
def __written_file(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	descriptor = rexlib.make_contiguous_array_descriptor(
		(4, 6), rexlib.NumericalType.float32
	)
	image.write(
		rexlib.zeros(
			descriptor,
			rexlib.hardware.MemoryResourceAffinity.host,
			__setup_context
		),
		path
	)
	return path

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	return rexlib.dispatch.ExecutionContext(device_context, dispatcher)
