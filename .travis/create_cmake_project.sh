#!/bin/sh

# This script will create the cmake sirius project build directory
# Usage: ./create_cmake_project.sh BUILD_DIR INSTALL_DIR SIRIUS_VERSION SIRIUS_REVISION_COMMIT

BUILD_DIR=$1
INSTALL_DIR=$2
SIRIUS_VERSION=$3
SIRIUS_REVISION_COMMIT=$4

[ ! -d ${BUILD_DIR} ] && mkdir ${BUILD_DIR};
cd ${BUILD_DIR};

cmake3 .. -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
          -DSIRIUS_VERSION="${SIRIUS_VERSION}" \
          -DSIRIUS_REVISION_COMMIT="${SIRIUS_REVISION_COMMIT}" \
          -DENABLE_CACHE_OPTIMIZATION=ON \
          -DENABLE_GSL_CONTRACTS=OFF \
          -DENABLE_LOGS=ON \
          -DENABLE_UNIT_TESTS=ON \
          -DENABLE_DOCUMENTATION=ON
cd ..