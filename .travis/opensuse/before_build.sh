#!/bin/bash

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

cd ${TRAVIS_BUILD_DIR}

SIRIUS_VERSION=$(git describe --tags 2> /dev/null || echo "")
if [[ "x${SIRIUS_VERSION}" = "x" ]]; then
    SIRIUS_VERSION="0.0.0"
fi
export SIRIUS_VERSION

# before script
export PROJECT_DIR=${TRAVIS_BUILD_DIR}
export INSTALL_DIR=${PROJECT_DIR}/install_directory
mkdir -p ${INSTALL_DIR}