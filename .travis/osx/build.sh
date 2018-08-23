#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'osx' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

SIRIUS_REVISION_COMMIT=${TRAVIS_COMMIT}
BUILD_DIR="${PROJECT_DIR}/.build"

echo "SIRIUS_VERSION=${SIRIUS_VERSION}"
echo "SIRIUS_REVISION_COMMIT=${SIRIUS_REVISION_COMMIT}"
echo "PROJECT_DIR=${PROJECT_DIR}"
echo "INSTALL_DIR=${INSTALL_DIR}"
echo "BUILD_DIR=${BUILD_DIR}"

[ ! -d ${BUILD_DIR} ] && mkdir -p ${BUILD_DIR};
cd ${BUILD_DIR}

# create makefile project
cmake ${PROJECT_DIR} \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DSIRIUS_VERSION="${SIRIUS_VERSION}" \
    -DSIRIUS_REVISION_COMMIT="${SIRIUS_REVISION_COMMIT}" \
    -DENABLE_CACHE_OPTIMIZATION=ON \
    -DENABLE_GSL_CONTRACTS=ON \
    -DENABLE_LOGS=ON \
    -DENABLE_UNIT_TESTS=ON \
    -DENABLE_DOCUMENTATION=OFF

# build and run tests
make -j4 build_tests
${PROJECT_DIR}/.travis/run_tests.sh

# build executables
make -j4

# install project
make -j4 install