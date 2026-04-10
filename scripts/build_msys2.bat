@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Build Script with MSYS2
echo ========================================
echo.

set "PROJECT_NAME=PersonalDateAssisant"
set "BUILD_TYPE=Release"
set "SOURCE_DIR=%~dp0.."
set "BUILD_DIR=%SOURCE_DIR%\ReleaseTemp\build"

set "MSYS2_ROOT=C:\CodeSpace\Compile\Mysy2\ucrt64"
set "PATH=%MSYS2_ROOT%\bin;%PATH%"

echo [INFO] Using GCC from: %MSYS2_ROOT%\bin
echo [INFO] GCC Version:
%MSYS2_ROOT%\bin\g++.exe --version | findstr "g++"
echo.

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo [Step 1/3] Cleaning build directory...
if exist "%BUILD_DIR%" (
    echo Cleaning existing build directory...
    powershell -Command "Start-Sleep -Milliseconds 500; Remove-Item -Path '%BUILD_DIR%\*' -Recurse -Force -ErrorAction SilentlyContinue"
    timeout /t 2 /nobreak >nul
)
mkdir "%BUILD_DIR%"
echo Build directory ready
echo.

echo [Step 2/3] Configuring CMake project...
cd /d "%BUILD_DIR%"

cmake -G "MinGW Makefiles" ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX="%BUILD_DIR%\install" ^
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BUILD_DIR%" ^
      -DCMAKE_C_COMPILER="%MSYS2_ROOT%\bin\gcc.exe" ^
      -DCMAKE_CXX_COMPILER="%MSYS2_ROOT%\bin\g++.exe" ^
      "%SOURCE_DIR%"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed! Error code: %ERRORLEVEL%
    pause
    exit /b 1
)
echo Configuration completed
echo.

echo [Step 3/3] Building project...
mingw32-make -j4

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Error code: %ERRORLEVEL%
    pause
    exit /b 1
)
echo Build completed successfully!
echo.
echo [SUCCESS] Executable location: %BUILD_DIR%\%PROJECT_NAME%.exe
echo.

pause
