from __future__ import annotations
import collections.abc
import typing
import xmipp4._core_binding.numerical
__all__: list[str] = ['Array', 'ArrayDescriptor', 'is_initialized', 'make_contiguous_array_descriptor']
class Array:
    pass
class ArrayDescriptor:
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: ArrayDescriptor) -> bool:
        ...
    def __init__(self) -> None:
        ...
    def __ne__(self, arg0: ArrayDescriptor) -> bool:
        ...
    @property
    def data_type(self) -> xmipp4._core_binding.numerical.NumericalType:
        ...
def is_initialized(arg0: ArrayDescriptor) -> bool:
    ...
def make_contiguous_array_descriptor(extents: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], data_type: xmipp4._core_binding.numerical.NumericalType) -> ArrayDescriptor:
    ...
