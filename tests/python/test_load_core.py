# SPDX-License-Identifier: GPL-3.0-only

import ctypes
import pathlib
from importlib import metadata

# xmipp4.load_core is the re-exported function, so the module has to be
# reached explicitly.
from xmipp4 import load_core
from xmipp4.load_core import (
	_iter_distribution_library_paths as iter_distribution_library_paths,
)

class DistributionMock:
	def __init__(self, root, files):
		self.__root = root
		self.files = files

	def locate_file(self, path):
		return self.__root / path

def test_loads_the_core_library():
	assert isinstance(load_core(), ctypes.CDLL)

def test_finds_the_library_recorded_by_the_distribution(monkeypatch, tmp_path):
	library = tmp_path / 'bin' / 'libmock.so'
	library.parent.mkdir()
	library.touch()
	distribution = DistributionMock(
		tmp_path / 'lib' / 'site-packages',
		[pathlib.PurePosixPath('../../bin/libmock.so')]
	)
	monkeypatch.setattr(metadata, 'distribution', lambda name: distribution)

	found = list(iter_distribution_library_paths('mock', 'libmock.so'))
	assert found == [str(library)]

def test_ignores_files_that_are_not_the_library(monkeypatch, tmp_path):
	other = tmp_path / 'bin' / 'libother.so'
	other.parent.mkdir()
	other.touch()
	distribution = DistributionMock(
		tmp_path / 'lib' / 'site-packages',
		[pathlib.PurePosixPath('../../bin/libother.so')]
	)
	monkeypatch.setattr(metadata, 'distribution', lambda name: distribution)

	assert list(iter_distribution_library_paths('mock', 'libmock.so')) == []

def test_skips_recorded_files_that_do_not_exist(monkeypatch, tmp_path):
	distribution = DistributionMock(
		tmp_path / 'lib' / 'site-packages',
		[pathlib.PurePosixPath('../../bin/libmock.so')]
	)
	monkeypatch.setattr(metadata, 'distribution', lambda name: distribution)

	assert list(iter_distribution_library_paths('mock', 'libmock.so')) == []

def test_yields_nothing_when_the_distribution_is_not_installed():
	found = iter_distribution_library_paths(
		'a-distribution-that-is-not-installed', 'libmock.so'
	)
	assert list(found) == []

def test_yields_nothing_when_the_distribution_records_no_files(monkeypatch):
	distribution = DistributionMock(pathlib.Path('.'), None)
	monkeypatch.setattr(metadata, 'distribution', lambda name: distribution)

	assert list(iter_distribution_library_paths('mock', 'libmock.so')) == []
