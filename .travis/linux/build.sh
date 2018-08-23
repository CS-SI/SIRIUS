#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
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
echo "CXX_COMPILER=${CXX_COMPILER}"

[ ! -d ${BUILD_DIR} ] && mkdir -p ${BUILD_DIR};
cd ${BUILD_DIR}

# create makefile project
cmake ${PROJECT_DIR} \
    -G "Unix Makefiles" \
    -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DSIRIUS_VERSION="${SIRIUS_VERSION}" \
    -DSIRIUS_REVISION_COMMIT="${SIRIUS_REVISION_COMMIT}" \
    -DENABLE_CACHE_OPTIMIZATION=ON \
    -DENABLE_GSL_CONTRACTS=ON \
    -DENABLE_LOGS=ON \
    -DENABLE_UNIT_TESTS=ON \
    -DENABLE_DOCUMENTATION=OFF

# build sirius shared library
make -j4 libsirius

# build sirius static library
make -j4 libsirius-static

# build tests
make -j4 build_tests

# run tests
ctest --output-on-failure

# build sirius executable
make -j4 sirius

# execute sirius
./src/sirius -h

# install project
make -j4 install