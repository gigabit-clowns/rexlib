# SPDX-License-Identifier: GPL-3.0-only

import pickle

import pytest

import xmipp4

def test_constructor():
  v = xmipp4.Version(1234, 567, 890)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
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
  v1 = xmipp4.Version(major, minor, patch)
  v2 = xmipp4.Version(1, 2, 3)
  assert (v1 >= v2) == is_greater_or_equal

def test_version_to_string():
  v = xmipp4.Version(1234, 567, 890)
  assert str(v) == "1234.567.890"

def test_pickle():
  v = xmipp4.Version(1234, 567, 890)
  pickled = pickle.dumps(v)
  unpickled = pickle.loads(pickled)
  assert v == unpickled
