#!/bin/bash

[[ "${DEBUG}" == 'true' ]] && set -o xtrace

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" && pwd )"
ROOT_DIR="$CURRENT_DIR/.."

pushd "${ROOT_DIR}" > /dev/null

    echo "Generating .pyi stubs for xmipp4._core_binding..."
    OUT_DIR="$(mktemp -d)"
    python -m pybind11_stubgen xmipp4._core_binding \
        -o "${OUT_DIR}" \
        --ignore-invalid-expressions 'xmipp4::memory_allocator'
    STUBGEN_EXIT_CODE=$?
    if [ $STUBGEN_EXIT_CODE -ne 0 ]; then
        rm -rf "${OUT_DIR}"
        exit $STUBGEN_EXIT_CODE
    fi

    # MemoryResource.create_allocator() and MemoryAllocator.memory_resource()
    # reference each other, so no bind order avoids one of the two being
    # unresolved when pybind11 generates its docstring. We know the answer.
    sed -i 's/def create_allocator(self) -> \.\.\.:/def create_allocator(self) -> MemoryAllocator:/' \
        "${OUT_DIR}/xmipp4/_core_binding/hardware.pyi"

    # PluginManager.discover_plugins' default resolves
    # get_default_plugin_directory() at bind time, baking in an absolute,
    # machine-specific path into the stub.
    sed -i -E "s/directory: str = '[^']*'/directory: str = .../" \
        "${OUT_DIR}/xmipp4/_core_binding/__init__.pyi"

    # functional.*'s `out` parameters are `array *out = nullptr` in C++,
    # i.e. optional, but pybind11's docstring only records the `None`
    # default, not that the type itself is nullable, so stubgen infers a
    # non-Optional type with a `None` default (invalid under PEP 484).
    sed -i -E 's/out: ([A-Za-z0-9_.]+) = None/out: \1 | None = None/g' \
        "${OUT_DIR}/xmipp4/_core_binding/functional.pyi"

    rm -rf python/xmipp4/_core_binding
    cp -r "${OUT_DIR}/xmipp4/_core_binding" python/xmipp4/_core_binding
    rm -rf "${OUT_DIR}"

    echo "Stubs written to python/xmipp4/_core_binding/"

popd > /dev/null
