#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

DOCKER_BUILD_DIR=${TRAVIS_BUILD_DIR}/build-sirius-dockerfile
echo "TRAVIS_BUILD_DIR : ${TRAVIS_BUILD_DIR}"
echo "DOCKER_BUILD_DIR : ${DOCKER_BUILD_DIR}"
mkdir -p ${DOCKER_BUILD_DIR}
cd ${DOCKER_BUILD_DIR}

# docker install
cat >Dockerfile <<EOF
FROM opensuse/leap

# Install dependencies
RUN zypper -n install \
    vim \
    git \
    python \
    gcc \
    gcc-c++ \
    fftw3-devel \
    gdal-devel \
    python \
    cmake

EOF
