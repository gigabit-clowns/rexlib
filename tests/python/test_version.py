# SPDX-License-Identifier: GPL-3.0-only

import pickle
from importlib import metadata

import pytest

import rexlib

def test_constructor():
  v = rexlib.Version(1234, 567, 890)
  assert (
    (
      v.major, v.minor, v.patch
    ) == (1234, 567, 890)
  )

@pytest.mark.parametrize(
  "major, minor, patch, is_equal",
  [
    pytest.param(1, 2, 3, True, id="Equals"),
    pytest.param(2, 2, 3, False, id="Different major"),
    pytest.param(1, 1, 3, False, id="Different minor"),
    pytest.param(1, 2, 2, False, id="Different patch")
  ],
)
def test_versions_are_equal(major, minor, patch, is_equal):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 == v2) == is_equal

@pytest.mark.parametrize(
  "major, minor, patch, is_not_equal",
  [
    pytest.param(0, 2, 3, True, id="Different major"),
    pytest.param(1, 1, 3, True, id="Different minor"),
    pytest.param(1, 2, 2, True, id="Different patch"),
    pytest.param(1, 2, 3, False, id="Equals")
  ],
)
def test_versions_are_not_equal(major, minor, patch, is_not_equal):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 != v2) == is_not_equal

@pytest.mark.parametrize(
  "major, minor, patch, is_less",
  [
    pytest.param(0, 2, 3, True, id="Smaller major"),
    pytest.param(1, 1, 3, True, id="Smaller minor"),
    pytest.param(1, 2, 2, True, id="Smaller patch"),
    pytest.param(1, 2, 3, False, id="Equals"),
    pytest.param(2, 2, 3, False, id="Bigger major"),
    pytest.param(1, 3, 3, False, id="Bigger minor"),
    pytest.param(1, 2, 4, False, id="Bigger patch")
  ],
)
def test_version_is_less(major, minor, patch, is_less):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 < v2) == is_less

@pytest.mark.parametrize(
  "major, minor, patch, is_less_or_equal",
  [
    pytest.param(0, 2, 3, True, id="Smaller major"),
    pytest.param(1, 1, 3, True, id="Smaller minor"),
    pytest.param(1, 2, 2, True, id="Smaller patch"),
    pytest.param(1, 2, 3, True, id="Equals"),
    pytest.param(2, 2, 3, False, id="Bigger major"),
    pytest.param(1, 3, 3, False, id="Bigger minor"),
    pytest.param(1, 2, 4, False, id="Bigger patch")
  ],
)
def test_version_is_less_or_equal(major, minor, patch, is_less_or_equal):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 <= v2) == is_less_or_equal

@pytest.mark.parametrize(
  "major, minor, patch, is_greater",
  [
    pytest.param(0, 2, 3, False, id="Smaller major"),
    pytest.param(1, 1, 3, False, id="Smaller minor"),
    pytest.param(1, 2, 2, False, id="Smaller patch"),
    pytest.param(1, 2, 3, False, id="Equals"),
    pytest.param(2, 2, 3, True, id="Bigger major"),
    pytest.param(1, 3, 3, True, id="Bigger minor"),
    pytest.param(1, 2, 4, True, id="Bigger patch")
  ],
)
def test_version_is_greater(major, minor, patch, is_greater):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 > v2) == is_greater

@pytest.mark.parametrize(
  "major, minor, patch, is_greater_or_equal",
  [
    pytest.param(0, 2, 3, False, id="Smaller major"),
    pytest.param(1, 1, 3, False, id="Smaller minor"),
    pytest.param(1, 2, 2, False, id="Smaller patch"),
    pytest.param(1, 2, 3, True, id="Equals"),
    pytest.param(2, 2, 3, True, id="Bigger major"),
    pytest.param(1, 3, 3, True, id="Bigger minor"),
    pytest.param(1, 2, 4, True, id="Bigger patch")
  ],
)
def test_version_is_greater_or_equal(major, minor, patch, is_greater_or_equal):
  v1 = rexlib.Version(major, minor, patch)
  v2 = rexlib.Version(1, 2, 3)
  assert (v1 >= v2) == is_greater_or_equal

def test_version_to_string():
  v = rexlib.Version(1234, 567, 890)
  assert str(v) == "1234.567.890"

def test_pickle():
  v = rexlib.Version(1234, 567, 890)
  pickled = pickle.dumps(v)
  unpickled = pickle.loads(pickled)
  assert v == unpickled

def test_binding_version_is_the_distribution_version():
  # Not the C++ library's: the two are versioned independently.
  assert isinstance(rexlib.__version__, str)

  try:
    installed = metadata.version("rexlib")
  except metadata.PackageNotFoundError:
    pytest.skip("rexlib is on the path but not installed")
  assert rexlib.__version__ == installed

def test_reports_the_rexlib_version():
  # The C++ library's, versioned independently of this binding.
  v = rexlib.rexlib_version
  assert isinstance(v, rexlib.Version)
  assert (v.major, v.minor, v.patch) >= (0, 1, 0)
  assert str(v) == f"{v.major}.{v.minor}.{v.patch}"
