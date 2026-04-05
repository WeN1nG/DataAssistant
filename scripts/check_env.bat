@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Environment Check Tool v1.0
echo ========================================
echo.

set "ALL_CHECKS_PASSED=YES"

echo [Check 1/7] Detecting operating system...
ver | findstr /i "10\." >nul
if %ERRORLEVEL% EQU 0 (
    echo   [OK] Windows 10/11 detected
) else (
    echo   [FAIL] Windows 10 or higher required
    set "ALL_CHECKS_PASSED=NO"
)
echo.

echo [Check 2/7] Checking CMake...
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    cmake --version | findstr /r "^cmake"
    echo   [OK] CMake is installed
) else (
    echo   [FAIL] CMake not installed or not in PATH
    echo   Download: https://cmake.org/download/
    set "ALL_CHECKS_PASSED=NO"
)
echo.

echo [Check 3/7] Checking MinGW compiler...
where gcc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    gcc --version | findstr /r "gcc"
    echo   [OK] MinGW/GCC is installed
) else (
    echo   [FAIL] MinGW/GCC not installed or not in PATH
    echo   Recommended: Use Qt bundled MinGW toolchain
    set "ALL_CHECKS_PASSED=NO"
)
echo.

echo [Check 4/7] Checking Qt installation...
where qmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    qmake --version
    echo   [OK] Qt is installed
) else (
    echo   [FAIL] Qt not installed or not in PATH
    echo   Download: https://www.qt.io/download-qt-installer
    set "ALL_CHECKS_PASSED=NO"
)
echo.

echo [Check 5/7] Checking windeployqt...
where windeployqt >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   [OK] windeployqt is installed
) else (
    echo   [FAIL] windeployqt not installed or not in PATH
    echo   Make sure Qt bin directory is in PATH
    set "ALL_CHECKS_PASSED=NO"
)
echo.

echo [Check 6/7] Checking Inno Setup (optional)...
where iscc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   [OK] Inno Setup is installed
    echo   (Only needed for package.bat, can skip)
) else (
    echo   [WARN] Inno Setup not installed
    echo   (Only used for creating installer, can use portable.bat instead)
)
echo.

echo [Check 7/7] Checking disk space...
powershell -Command "Get-PSDrive C | Select-Object -ExpandProperty Free" > %TEMP%\diskspace.tmp
set /p DISKSPACE=<%TEMP%\diskspace.tmp
del %TEMP%\diskspace.tmp

if %DISKSPACE% GTR 500000 (
    echo   [OK] Sufficient disk space (about !DISKSPACE! MB free)
) else (
    echo   [WARN] Disk space may be insufficient (about !DISKSPACE! MB free)
    echo   Recommended: at least 500MB free space
)
echo.

echo ========================================
if "!ALL_CHECKS_PASSED!"=="YES" (
    echo [SUCCESS] All required checks passed!
    echo ========================================
    echo.
    echo Recommended next steps:
    echo 1. Run full packaging: package.bat
    echo 2. Or create portable version: portable.bat
    echo.
    echo For detailed instructions see:
    echo - QUICKSTART.md - Quick start guide
    echo - PACKAGING_GUIDE.md - Complete documentation
    echo.
) else (
    echo ========================================
    echo [WARNING] Some checks failed
    echo ========================================
    echo.
    echo Please install missing components:
    echo 1. Qt 6.x: https://www.qt.io/download-qt-installer
    echo 2. CMake: https://cmake.org/download/
    echo 3. MinGW: Recommended to use Qt bundled toolchain
    echo.
    echo After adding to PATH, run this script again
    echo.
)

echo ========================================
echo Environment check completed
echo ========================================
echo.

endlocal
pause
