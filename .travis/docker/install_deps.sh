#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

# docker install
docker pull ldumas/sirius_dockerfile:sirius_build
docker pull ldumas/sirius_dockerfile:sirius_pages
