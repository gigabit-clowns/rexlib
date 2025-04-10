#!/bin/bash

[[ "${DEBUG}" == 'true' ]] && set -o xtrace

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" && pwd )"
ROOT_DIR="$CURRENT_DIR/.."

pushd "${ROOT_DIR}" > /dev/null

    echo "Running tests..."
    python -m pytest -v --cache-clear --rootdir="${ROOT_DIR}"
    PYTEST_EXIT_CODE=$?
    if [ $PYTEST_EXIT_CODE -ne 0 ]; then
        exit $PYTEST_EXIT_CODE
    fi

popd > /dev/null
