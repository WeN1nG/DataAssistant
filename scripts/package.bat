@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Full Packaging Script v1.0
echo ========================================
echo.

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%~dp0.."
set "BUILD_DIR=%PROJECT_ROOT%\ReleaseTemp\build"
set "DEPLOY_DIR=%PROJECT_ROOT%\ReleaseTemp\deploy"
set "INSTALLER_DIR=%PROJECT_ROOT%\ReleaseTemp\installer"

echo [Phase 1/4] Cleaning old build files...
call :cleanup_directory "%PROJECT_ROOT%\ReleaseTemp"
echo Cleanup completed
echo.

echo [Phase 2/4] Building project...
echo ========================================
call "%SCRIPT_DIR%build.bat"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed, packaging aborted!
    echo.
    pause
    exit /b 1
)
echo Build phase completed
echo.

echo [Phase 3/4] Deploying Qt runtime dependencies...
echo ========================================
call "%SCRIPT_DIR%deploy.bat"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Qt dependency deployment failed, packaging aborted!
    echo.
    pause
    exit /b 1
)
echo Deployment phase completed
echo.

echo [Phase 4/4] Creating installer...
echo ========================================
if not exist "%INSTALLER_DIR%" (
    mkdir "%INSTALLER_DIR%"
)

where iscc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [WARNING] Inno Setup compiler not found!
    echo.
    echo Manual compilation steps:
    echo 1. Download and install Inno Setup: https://jrsoftware.org/isinfo.php
    echo 2. Open "%SCRIPT_DIR%installer.iss"
    echo 3. Press Ctrl+F9 to compile installer
    echo.
    echo Output files will be:
    echo - %INSTALLER_DIR%\DataAssistant_Setup_v0.1.exe
    echo.
    echo You can also use portable packaging (run portable.bat)
    pause
    exit /b 0
)

cd /d "%SCRIPT_DIR%"
iscc installer.iss

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Inno Setup compilation failed!
    echo Please check installer.iss script syntax
    pause
    exit /b 1
)
echo Installer created
echo.

echo ========================================
echo Packaging completed successfully!
echo ========================================
echo.
echo Generated files in ReleaseTemp directory:
echo 1. Build output: %BUILD_DIR%
echo 2. Deployment: %DEPLOY_DIR%
echo 3. Installer: %INSTALLER_DIR%
echo.
echo ReleaseTemp structure:
echo ReleaseTemp/
echo   - build/          [Compiled executable]
echo   - deploy/         [Deployment package with Qt dependencies]
echo   - installer/      [Windows installer (if Inno Setup installed)]
echo   - portable/       [Portable version (if created)]
echo.
echo Next steps:
echo 1. Test the application from: %DEPLOY_DIR%
echo 2. Run Windows Defender scan to verify security
echo 3. Perform compatibility testing on Windows 10/11
echo ========================================
echo.

cd /d "%PROJECT_ROOT%"
endlocal
exit /b 0

:cleanup_directory
set "TARGET_DIR=%~1"
if exist "%TARGET_DIR%" (
    rmdir /s /q "%TARGET_DIR%" 2>nul
    if exist "%TARGET_DIR%" (
        powershell -Command "Remove-Item -Path '%TARGET_DIR%' -Recurse -Force -ErrorAction SilentlyContinue"
    )
)
goto :eof
