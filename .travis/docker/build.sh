#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

# docker script
docker run -v $TRAVIS_BUILD_DIR/:/data ldumas/sirius_dockerfile:sirius_build /bin/sh -c "source /opt/rh/devtoolset-6/enable; ${PROJECT_DIR}/.travis/docker/build_install_sirius.sh ${PROJECT_DIR} ${INSTALL_DIR} ${SIRIUS_VERSION} ${TRAVIS_COMMIT}" ${DEPLOY}
export BUILD_CONTAINER_ID=$(docker ps --latest --quiet)