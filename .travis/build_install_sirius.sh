#!/bin/sh

# This script will build sirius project (library, binary and unit tests)
# Usage: ./build_sirius.sh PROJECT_DIR INSTALL_DIR SIRIUS_VERSION SIRIUS_REVISION_COMMIT

set -ev

PROJECT_DIR=$1
INSTALL_DIR=$2
SIRIUS_VERSION=$3
SIRIUS_REVISION_COMMIT=$4

BUILD_DIR="${PROJECT_DIR}/.build"

cd ${PROJECT_DIR}

.travis/create_cmake_project.sh ${BUILD_DIR} ${INSTALL_DIR} ${SIRIUS_VERSION} ${SIRIUS_REVISION_COMMIT}

cd ${BUILD_DIR}
make -j4 build_tests

${PROJECT_DIR}/.travis/run_tests.sh

make -j4

make install
cd ${PROJECT_DIR}
mv "${BUILD_DIR}/html" doxy_html
mv "${BUILD_DIR}/xml" doxy_xml
rm -rf ${BUILD_DIR}