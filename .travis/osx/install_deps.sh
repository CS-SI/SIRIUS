#!/bin/bash

set -ev

if [[ ${TRAVIS_OS_NAME} != 'osx' ]]; then
    # unsupported platform
    echo "unsupported platform"
    exit 22
fi

# os x install

# the following line should resolve the error:
#   Possible conflicting files are:
# /usr/local/include/c++ -> /usr/local/Caskroom/oclint/0.13.1,17.4.0/oclint-0.13.1/include/c++
brew cask uninstall oclint

brew update

# try to update gdal
if brew outdated --quiet | grep -qx gdal; then brew upgrade gdal || true; fi

# install fftw
brew install fftw
