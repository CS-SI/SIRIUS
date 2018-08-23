#!/bin/bash

# This script will build sirius project (library, binary and unit tests) and rrun unit tests
# Usage: ./build_sirius.sh PROJECT_DIR INSTALL_DIR SIRIUS_VERSION SIRIUS_REVISION_COMMIT

set -ev

PROJECT_DIR=$1
INSTALL_DIR=$2
SIRIUS_VERSION=$3
SIRIUS_REVISION_COMMIT=$4

BUILD_DIR="${PROJECT_DIR}/.build"

echo "SIRIUS_VERSION=${SIRIUS_VERSION}"
echo "SIRIUS_REVISION_COMMIT=${SIRIUS_REVISION_COMMIT}"
echo "PROJECT_DIR=${PROJECT_DIR}"
echo "INSTALL_DIR=${INSTALL_DIR}"
echo "BUILD_DIR=${BUILD_DIR}"

[ ! -d ${BUILD_DIR} ] && mkdir -p ${BUILD_DIR};
cd ${BUILD_DIR}

# cmake3 is used instead of cmake due to CentOS docker image
cmake3 ${PROJECT_DIR} \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DSIRIUS_VERSION="${SIRIUS_VERSION}" \
    -DSIRIUS_REVISION_COMMIT="${SIRIUS_REVISION_COMMIT}" \
    -DENABLE_CACHE_OPTIMIZATION=ON \
    -DENABLE_GSL_CONTRACTS=OFF \
    -DENABLE_LOGS=ON \
    -DENABLE_UNIT_TESTS=ON \
    -DENABLE_DOCUMENTATION=ON

# build and run tests
make -j4 build_tests
${PROJECT_DIR}/.travis/run_tests.sh

# build executables
make -j4

# install project
make -j4 install

cd ${PROJECT_DIR}
mv "${BUILD_DIR}/html" doxy_html
mv "${BUILD_DIR}/xml" doxy_xml
rm -rf ${BUILD_DIR}