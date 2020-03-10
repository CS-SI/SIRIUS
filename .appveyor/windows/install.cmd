@echo on

cd %VCPKG_DIR%

appveyor AddMessage "Updating vcpkg knowledge base..."
:: update vcpkg knowledge base
git pull

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

appveyor AddMessage "Installing PROJ4 dependency..."
vcpkg install proj4:%VCPKG_TRIPLET% || goto error

appveyor AddMessage "Installing GDAL dependency..."
vcpkg install gdal[core,mysql-libmariadb]:%VCPKG_TRIPLET% || goto error
appveyor AddMessage "Installing GDAL dependency done"

exit /b 0

:error
set cmd_errorlevel=%errorlevel%
appveyor AddMessage "Dependencies were not generated. Check the logs and fix vcpkg generation" -Category Error
exit /b %cmd_errorlevel%