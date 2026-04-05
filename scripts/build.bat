@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Build Script v1.0
echo ========================================
echo.

set "PROJECT_NAME=PersonalDateAssisant"
set "BUILD_TYPE=Release"
set "SOURCE_DIR=%~dp0.."
set "BUILD_DIR=%SOURCE_DIR%\ReleaseTemp\build"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo [Step 1/3] Cleaning build directory...
if exist "%BUILD_DIR%\*" (
    rmdir /s /q "%BUILD_DIR%" 2>nul
    if exist "%BUILD_DIR%" (
        echo Warning: Force cleaning build directory...
        powershell -Command "Remove-Item -Path '%BUILD_DIR%\*' -Recurse -Force -ErrorAction SilentlyContinue"
    )
)
mkdir "%BUILD_DIR%"
echo Build directory cleaned
echo.

echo [Step 2/3] Configuring CMake project...
cd /d "%BUILD_DIR%"

cmake -G "MinGW Makefiles" ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX="%BUILD_DIR%\install" ^
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%" ^
      "%SOURCE_DIR%"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed! Error code: %ERRORLEVEL%
    echo Possible solutions:
    echo 1. Make sure Qt6 and MinGW compiler are installed
    echo 2. Check if Qt path is configured correctly
    echo 3. See error messages above for details
    pause
    exit /b 1
)
echo Configuration completed
echo.

echo [Step 3/3] Building project...
cmake --build . --config %BUILD_TYPE% -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Error code: %ERRORLEVEL%
    echo Possible solutions:
    echo 1. Check source code for syntax errors
    echo 2. Make sure all dependencies are installed
    echo 3. See build error messages above
    pause
    exit /b 1
)
echo Build completed
echo.

echo ========================================
echo Build SUCCESS!
echo Executable location: %BUILD_DIR%\%PROJECT_NAME%.exe
echo ========================================
echo.

cd /d "%SOURCE_DIR%"
endlocal
