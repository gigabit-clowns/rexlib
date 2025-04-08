import ctypes
import os
import platform
import sys
from typing import List, Generator

def __get_library_filename(name: str, system: str) -> str:
    """Get the library filename based on the operating system."""
    if system == 'Darwin':
        return f"lib{name}.dylib"
    elif system == 'Windows':
        return f"{name}.dll"
    else: # Linux or other Unix-like systems
        return f"lib{name}.so"

def __get_library_directory_names(system) -> List[str]:
    """Get the library directory names based on the operating system."""
    if system == 'Windows':
        return ["bin"]
    else: # Linux, MacOS, or other Unix-like systems
        return ["lib", "lib64"]

def __iter_possible_library_paths(prefix: str, 
                                  filename: str, 
                                  system: str ) -> Generator[str, None, None]:
    """Iterate over possible paths for the library."""
    yield filename
    for libdir in __get_library_directory_names(system):
        path = os.path.join(prefix, libdir, filename)
        if os.path.exists(path):
            yield path

def __load_library(name: str) -> ctypes.CDLL:
    """Heuristically find and load a library with the specified name."""
    system = platform.system()
    filename = __get_library_filename(name, system)
    for dynamic_lib in __iter_possible_library_paths(sys.prefix, filename, system):
        try:
            return ctypes.CDLL(dynamic_lib)
        except OSError:
            continue
    
    raise OSError(f"Could not find {name}.")

def load_core() -> ctypes.CDLL:
    """
    Load the core library for xmipp4.
    
    This function attempts to load the xmipp4 shared object from the system's
    library directories. It raises an exception if the library cannot be found.
    
    Returns:
        ctypes.CDLL: The loaded core library.
    """
    return __load_library("xmipp4-core")
