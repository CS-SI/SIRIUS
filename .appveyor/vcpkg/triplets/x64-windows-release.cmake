# This vcpkg triplet is specialized to reduce build time for SIRIUS dependencies
# Appveyor build timeout is 1h and gdal and its dependencies take a long time to build...

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# do not build debug version for packages (save space and build time for CI)
# see also: https://github.com/Microsoft/vcpkg/issues/143
set(VCPKG_BUILD_TYPE release)