# SPDX-License-Identifier: GPL-3.0-only

import pickle

import pytest

import xmipp4

def test_constructor_from_params():
  xmipp4.compute.DeviceIndex('host', 0) is not None

def test_constructor_from_string():
  xmipp4.compute.DeviceIndex('cuda:0') is not None

@pytest.mark.parametrize(
  "backend, device_id",
  [
    pytest.param("host", 0),
    pytest.param("host", 1),
    pytest.param("cuda", 0),
    pytest.param("cuda", 1)
  ]
)
def test_returns_expected_properties(backend, device_id):
  device_index = xmipp4.compute.DeviceIndex(backend, device_id)
  assert (
    (
      device_index.backend, device_index.id
    ) == (backend, device_id)
  )

@pytest.mark.parametrize(
  "backend, device_id",
  [
    pytest.param("host", "2", id="Passing a string as id"),
    pytest.param(2, 1, id="Passing a number as backend"),
    pytest.param(None, 1, id="Passing a none as backend"),
    pytest.param("cuda", None, id="Passing a none as id"),
  ]
)
def test_raises_type_error_with_invalid_arguments(backend, device_id):
  with pytest.raises(TypeError):
    xmipp4.compute.DeviceIndex(backend, device_id)
    
@pytest.mark.parametrize(
  "backend, device_id",
  [
    pytest.param("host", 0),
    pytest.param("host", 1),
    pytest.param("cuda", 0),
    pytest.param("cuda", 1)
  ]
)
def test_returns_expected_properties_from_string(backend, device_id):
  device_index = xmipp4.compute.DeviceIndex(f"{backend}:{device_id}")
  assert (
    (
      device_index.backend, device_index.id
    ) == (backend, device_id)
  )

@pytest.mark.parametrize(
  "name",
  [
    pytest.param("", id="Empty string"),
    pytest.param("cuda:something", id="Invalid id"),
    pytest.param(":2", id="Empty backend"),
    pytest.param("host:", id="Empty id"),
    pytest.param("host::2", id="Duplicated separators right side"),
    pytest.param(":host:2", id="Duplicated separators left side")
  ]
)
def test_raises_value_error_with_invalid_string(name):
  with pytest.raises(ValueError):
    xmipp4.compute.DeviceIndex(name)

@pytest.mark.parametrize(
  "backend, device_id, are_equals",
  [
    pytest.param("host", 0, False, id="With different backend"),
    pytest.param("cuda", 0, False, id="With different id"),
    pytest.param("cuda", 1, True, id="Equals")
  ]
)
def test_are_equals(backend, device_id, are_equals):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex('cuda', 1)
  assert (d1 == d2) == are_equals

@pytest.mark.parametrize(
  "backend, device_id, are_not_equals",
  [
    pytest.param("host", 0, True, id="With different id"),
    pytest.param("cuda", 0, True, id="With different backend"),
    pytest.param("host", 1, False, id="Equals")
  ]
)
def test_are_not_equals(backend, device_id, are_not_equals):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex('host', 1)
  assert (d1 != d2) == are_not_equals

@pytest.mark.parametrize(
  "backend, device_id, is_less",
  [
    pytest.param("host", 0, True, id="With smaller id"),
    pytest.param("cuda", 0, True, id="With alphabetically smaller backend & smaller id"),
    pytest.param("cuda", 1, True, id="With alphabetically smaller backend"),
    pytest.param("cuda", 2, True, id="With alphabetically smaller backend & bigger id"),
    pytest.param("host", 1, False, id="Equals"),
    pytest.param("host", 2, False, id="With bigger id"),
    pytest.param("metal", 1, False, id="With alphabetically bigger backend"),
    pytest.param("metal", 0, False, id="With alphabetically bigger backend & smaller id"),
    pytest.param("metal", 2, False, id="With alphabetically bigger backend & bigger id")
  ]
)
def test_is_less(backend, device_id, is_less):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex("host", 1)
  assert (d1 < d2) == is_less

@pytest.mark.parametrize(
  "backend, device_id, is_less_or_equal",
  [
    pytest.param("host", 0, True, id="With smaller id"),
    pytest.param("cuda", 0, True, id="With alphabetically smaller backend & smaller id"),
    pytest.param("cuda", 1, True, id="With alphabetically smaller backend"),
    pytest.param("cuda", 2, True, id="With alphabetically smaller backend & bigger id"),
    pytest.param("host", 1, True, id="Equals"),
    pytest.param("host", 2, False, id="With bigger id"),
    pytest.param("metal", 1, False, id="With alphabetically bigger backend"),
    pytest.param("metal", 0, False, id="With alphabetically bigger backend & smaller id"),
    pytest.param("metal", 2, False, id="With alphabetically bigger backend & bigger id")
  ]
)
def test_is_less_or_equal(backend, device_id, is_less_or_equal):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex("host", 1)
  assert (d1 <= d2) == is_less_or_equal

@pytest.mark.parametrize(
  "backend, device_id, is_greater",
  [
    pytest.param("host", 0, False, id="With smaller id"),
    pytest.param("cuda", 0, False, id="With alphabetically smaller backend & smaller id"),
    pytest.param("cuda", 1, False, id="With alphabetically smaller backend"),
    pytest.param("cuda", 2, False, id="With alphabetically smaller backend & bigger id"),
    pytest.param("host", 1, False, id="Equals"),
    pytest.param("host", 2, True, id="With bigger id"),
    pytest.param("metal", 1, True, id="With alphabetically bigger backend"),
    pytest.param("metal", 0, True, id="With alphabetically bigger backend & smaller id"),
    pytest.param("metal", 2, True, id="With alphabetically bigger backend & bigger id")
  ]
)
def test_is_greater(backend, device_id, is_greater):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex("host", 1)
  assert (d1 > d2) == is_greater

@pytest.mark.parametrize(
  "backend, device_id, is_greater_or_equal",
  [
    pytest.param("host", 0, False, id="With smaller id"),
    pytest.param("cuda", 0, False, id="With alphabetically smaller backend & smaller id"),
    pytest.param("cuda", 1, False, id="With alphabetically smaller backend"),
    pytest.param("cuda", 2, False, id="With alphabetically smaller backend & bigger id"),
    pytest.param("host", 1, True, id="Equals"),
    pytest.param("host", 2, True, id="With bigger id"),
    pytest.param("metal", 1, True, id="With alphabetically bigger backend"),
    pytest.param("metal", 0, True, id="With alphabetically bigger backend & smaller id"),
    pytest.param("metal", 2, True, id="With alphabetically bigger backend & bigger id")
  ]
)
def test_is_greater_or_equal(backend, device_id, is_greater_or_equal):
  d1 = xmipp4.compute.DeviceIndex(backend, device_id)
  d2 = xmipp4.compute.DeviceIndex("host", 1)
  assert (d1 >= d2) == is_greater_or_equal

@pytest.mark.parametrize(
  "backend, device_id",
  [
    pytest.param("host", 0),
    pytest.param("host", 1),
    pytest.param("cuda", 0),
    pytest.param("cuda", 1)
  ]
)
def test_device_index_to_string(backend, device_id):
  assert (
    str(
      xmipp4.compute.DeviceIndex(backend, device_id)
    ) == f"{backend}:{device_id}"
  )

@pytest.mark.parametrize(
  "backend, device_id",
  [
    pytest.param("host", 0),
    pytest.param("host", 1),
    pytest.param("cuda", 0),
    pytest.param("cuda", 1)
  ]
)
def test_device_index_repr(backend, device_id):
  assert (
    repr(
      xmipp4.compute.DeviceIndex(backend, device_id)
    ) == f"DeviceIndex(backend=\"{backend}\", id={device_id})"
  )

def test_pickle():
  di = xmipp4.compute.DeviceIndex("host", 0)
  pickled = pickle.dumps(di)
  unpickled = pickle.loads(pickled)
  assert di == unpickled
