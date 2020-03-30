#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi
# docker install
docker pull tfeuvrier/sirius.opensuse.leap:secondtry

# -------------------------------------------------------------------------
# Note: tfeuvrier/sirius.opensuse.leap docker image created with
#       following command lines:

#cat >Dockerfile <<EOF
#FROM opensuse/leap
#RUN zypper -n install \
#    python \
#    gcc \
#    gcc-c++ \
#    fftw3-devel \
#    gdal-devel \
#    python \
#    cmake
#EOF
#docker build -t  sirius.opensuse.leap .
#docker tag "name" tfeuvrier/sirius.opensuse.leap:secondtry
#docker push tfeuvrier/sirius.opensuse.leap