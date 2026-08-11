from __future__ import annotations
import typing
__all__: list[str] = ['NumericalType']
class NumericalType:
    """
    Members:
    
      boolean
    
      char8
    
      int8
    
      uint8
    
      int16
    
      uint16
    
      int32
    
      uint32
    
      int64
    
      uint64
    
      float16
    
      float32
    
      float64
    
      complex_float16
    
      complex_float32
    
      complex_float64
    """
    __members__: typing.ClassVar[dict[str, NumericalType]]  # value = {'boolean': <NumericalType.boolean: 0>, 'char8': <NumericalType.char8: 1>, 'int8': <NumericalType.int8: 2>, 'uint8': <NumericalType.uint8: 3>, 'int16': <NumericalType.int16: 4>, 'uint16': <NumericalType.uint16: 5>, 'int32': <NumericalType.int32: 6>, 'uint32': <NumericalType.uint32: 7>, 'int64': <NumericalType.int64: 8>, 'uint64': <NumericalType.uint64: 9>, 'float16': <NumericalType.float16: 10>, 'float32': <NumericalType.float32: 11>, 'float64': <NumericalType.float64: 12>, 'complex_float16': <NumericalType.complex_float16: 13>, 'complex_float32': <NumericalType.complex_float32: 14>, 'complex_float64': <NumericalType.complex_float64: 15>}
    boolean: typing.ClassVar[NumericalType]  # value = <NumericalType.boolean: 0>
    char8: typing.ClassVar[NumericalType]  # value = <NumericalType.char8: 1>
    complex_float16: typing.ClassVar[NumericalType]  # value = <NumericalType.complex_float16: 13>
    complex_float32: typing.ClassVar[NumericalType]  # value = <NumericalType.complex_float32: 14>
    complex_float64: typing.ClassVar[NumericalType]  # value = <NumericalType.complex_float64: 15>
    float16: typing.ClassVar[NumericalType]  # value = <NumericalType.float16: 10>
    float32: typing.ClassVar[NumericalType]  # value = <NumericalType.float32: 11>
    float64: typing.ClassVar[NumericalType]  # value = <NumericalType.float64: 12>
    int16: typing.ClassVar[NumericalType]  # value = <NumericalType.int16: 4>
    int32: typing.ClassVar[NumericalType]  # value = <NumericalType.int32: 6>
    int64: typing.ClassVar[NumericalType]  # value = <NumericalType.int64: 8>
    int8: typing.ClassVar[NumericalType]  # value = <NumericalType.int8: 2>
    uint16: typing.ClassVar[NumericalType]  # value = <NumericalType.uint16: 5>
    uint32: typing.ClassVar[NumericalType]  # value = <NumericalType.uint32: 7>
    uint64: typing.ClassVar[NumericalType]  # value = <NumericalType.uint64: 9>
    uint8: typing.ClassVar[NumericalType]  # value = <NumericalType.uint8: 3>
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
