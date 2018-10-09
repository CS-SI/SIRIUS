@echo on

cd %VCPKG_DIR%

appveyor AddMessage "Updating vcpkg knowledge base..."
:: update vcpkg knowledge base
::   git pull

:: TODO: remove after merges of vcpkg PR #4413 (geos) and #4433 (gdal)
:: use fork to fix multiple build issues with release only version of the dependencies
git config --global user.email "test@test.com"
git config --global user.name "Test Test"
git remote add release-build https://github.com/hlysunnaram/vcpkg.git || goto error
git fetch release-build update/gdal-2.3.2 || goto error
git fetch release-build update/geos-3.6.3 || goto error
git checkout update/gdal-2.3.2 || goto error
git merge release-build/update/geos-3.6.3 || goto error

:: need to execute bootstrapping in a new process since it exits the current cmd process at the end of its execution...
cmd.exe /C "bootstrap-vcpkg.bat -disableMetrics" || goto error
appveyor AddMessage "Updating vcpkg knowledge base done"

:: force release version of the dependencies with vcpkg triplet (VCPKG_BUILD_TYPE="release") to
::   * reduce compilation time (appveyor timeout is 1h)
::   * reduce disk space for dependencies cache
copy "%APPVEYOR_BUILD_FOLDER%\.appveyor\vcpkg\triplets\%VCPKG_TRIPLET%.cmake" "%VCPKG_DIR%\triplets" || goto error
:: install fftw3 and gdal packages
:: force x64 build due to gdal package only available for x64 (libmysql only available for x64)
appveyor AddMessage "Installing FFTW3 dependency..."
vcpkg install fftw3:%VCPKG_TRIPLET% || goto error
appveyor AddMessage "Installing FFTW3 dependency done"

appveyor AddMessage "Installing GDAL dependency..."
vcpkg install gdal[core,mysql-libmariadb]:%VCPKG_TRIPLET% || goto error
appveyor AddMessage "Installing GDAL dependency done"

exit /b 0

:error
set cmd_errorlevel=%errorlevel%
appveyor AddMessage "Dependencies were not generated. Check the logs and fix vcpkg generation" -Category Error
exit /b %cmd_errorlevel%