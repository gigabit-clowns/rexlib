# rexlib

[![CI](https://github.com/gigabit-clowns/rexlib/actions/workflows/ci.yml/badge.svg)](https://github.com/gigabit-clowns/rexlib/actions/workflows/ci.yml)

rexlib is a high-performance Cryo-EM and Cryo-ET image processing library,
written in C++ with a Python binding. Its operations run on whichever device a
backend plugin provides: the CPU backend is built in, and a CUDA plugin runs
them on NVIDIA GPUs.

## Installing

```bash
pip install rexlib
```

Wheels are built for Python 3.11 and later on Linux x86_64 and aarch64, macOS
arm64, and Windows x86_64 and arm64. On Linux, the CUDA plugin comes as an
extra, for CUDA 12 or CUDA 13:

```bash
pip install "rexlib[cuda12]"
```

The C++ library is installed from source with CMake, as below, and the wheel
carries its headers and CMake package too.

## Using it from Python

```python
import rexlib

descriptor = rexlib.make_contiguous_array_descriptor(
    [2, 2], rexlib.NumericalType.float32
)
zeros = rexlib.zeros(descriptor, rexlib.hardware.MemoryResourceAffinity.host)

with rexlib.device("cuda"):
    on_gpu = rexlib.zeros(
        descriptor, rexlib.hardware.MemoryResourceAffinity.device
    )
```

The default catalog loads every installed plugin wheel on first use.

## Highlights

- **Cryo-EM and Cryo-ET operations** in `rexlib::em` and `rexlib.em`, beside
  the array operations they are built on.
- **Device-agnostic execution model.** Operations are dispatched through
  services and execution contexts instead of being tied to one device API.
- **Pluggable backends.** A backend plugin is a shared object defined with
  `REXLIB_DEFINE_PLUGIN`, which the core loads only if it was built for a
  compatible rexlib and C++ ABI. A third party builds one against the
  installed library; the
  [plugin guide](https://github.com/gigabit-clowns/rexlib/blob/main/docs/agents/plugins.md)
  says how.
- **Public headers as contract.** The public interface is the headers in
  `include/rexlib/`, documented directly in the C++ reference, and consumable
  from C++14.

## Components

- `core`: operation model, dispatch, hardware abstraction, plugin loading,
  layouts and numerical infrastructure.
- `ops`: operation declarations.
- `functional`: user-facing functions that reach operation declarations.
- `em`: Cryo-EM and Cryo-ET operations and functions in `rexlib::em`.
- `backends/cpu`: the CPU backend.
- `python/`: the Python binding.
- `plugins/cuda/`: the CUDA plugin.

Image I/O is out of scope for rexlib and lives in
[vitrio](https://github.com/gigabit-clowns/vitrio).

## Using it from CMake

```cmake
find_package(rexlib REQUIRED)
target_link_libraries(my_program PRIVATE rexlib::rexlib)
```

## Minimal C++ example

```cpp
#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/program_service.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_index.hpp>
#include <rexlib/core/hardware/device_service.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/functional/creation.hpp>

#include <cstdint>
#include <iostream>

int main()
{
	std::cout << "rexlib " << rexlib::get_library_version() << "\n";

	rexlib::service_catalog catalog;
	const auto devices =
		catalog.get_service<rexlib::device_service>();
	const auto programs =
		catalog.get_service<rexlib::program_service>();
	const rexlib::execution_context context(
		rexlib::device_context(
			devices->create_device_session(rexlib::device_index("cpu", 0))
		),
		rexlib::make_eager_dispatcher(programs)
	);

	auto result = rexlib::arange(
		5,
		rexlib::numerical_type::int32,
		rexlib::memory_resource_affinity::host,
		context
	);

	const auto *values = static_cast<const std::int32_t*>(
		result.get_storage()->get_host_ptr()
	);
	for (std::int32_t i = 0; i < 5; ++i)
	{
		if (values[i] != i)
		{
			return 1;
		}
	}

	return 0;
}
```

## Building from source

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
```

With CMake 3.21 or newer, presets provide the same flow:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

With `-DREXLIB_BUILD_PYTHON=ON` the build includes the Python binding, and
`pip install .` builds the `rexlib` wheel. The CUDA plugin, in `plugins/cuda/`,
is built against an installed rexlib, as any plugin is. The
[build guide](https://github.com/gigabit-clowns/rexlib/blob/main/docs/agents/building.md)
has the details.

To generate the C++ API reference:

```bash
cmake -S . -B build -DREXLIB_BUILD_DOCS=ON -DBUILD_TESTING=OFF
cmake --build build --target rexlib-docs
```

## SonarCloud status
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)

### Ratings
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)

### Specific metrics
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=bugs)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=gigabit-clowns_rexlib&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=gigabit-clowns_rexlib)
