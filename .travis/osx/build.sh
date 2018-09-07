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
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DSIRIUS_VERSION="${SIRIUS_VERSION}" \
    -DSIRIUS_REVISION_COMMIT="${SIRIUS_REVISION_COMMIT}" \
    -DENABLE_CACHE_OPTIMIZATION=ON \
    -DENABLE_GSL_CONTRACTS=ON \
    -DENABLE_LOGS=ON \
    -DBUILD_TESTING=ON \
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
 ./src/sirius -v trace -r 2 ../data/input/lena.jpg lena_z2.tif

# install project
make -j4 install

# test find_package(SIRIUS)
mkdir ${PROJECT_DIR}/.build-test-find-package
cd ${PROJECT_DIR}/.build-test-find-package
cmake ../tests/find-package-test \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DSIRIUS_ROOT=${INSTALL_DIR} \
    -DENABLE_FIND_PACKAGE_SIRIUS_CONFIG=OFF
make -j4
./basic_sirius_static_test

# test find_package(SIRIUS CONFIG)
mkdir ${PROJECT_DIR}/.build-test-find-package-config
cd ${PROJECT_DIR}/.build-test-find-package-config
cmake ../tests/find-package-test \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DSIRIUS_DIR=${INSTALL_DIR}/share/cmake \
    -DENABLE_FIND_PACKAGE_SIRIUS_CONFIG=ON
make -j4
./basic_sirius_static_test