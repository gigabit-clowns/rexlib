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

    rm -rf python/xmipp4/_core_binding
    cp -r "${OUT_DIR}/xmipp4/_core_binding" python/xmipp4/_core_binding
    rm -rf "${OUT_DIR}"

    echo "Stubs written to python/xmipp4/_core_binding/"

popd > /dev/null
