#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

# docker script
docker run -v $TRAVIS_BUILD_DIR/:/data tfeuvrier/sirius.opensuse.leap:secondtry /bin/sh -c "${PROJECT_DIR}/.travis/opensuse/build_install_sirius.sh ${PROJECT_DIR} ${INSTALL_DIR} ${SIRIUS_VERSION} ${TRAVIS_COMMIT}" ${DEPLOY}
