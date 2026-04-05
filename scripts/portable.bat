@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Portable Packaging v1.0
echo ========================================
echo.

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%~dp0.."
set "BUILD_DIR=%PROJECT_ROOT%\ReleaseTemp\build"
set "DEPLOY_DIR=%PROJECT_ROOT%\ReleaseTemp\deploy"
set "PORTABLE_DIR=%PROJECT_ROOT%\ReleaseTemp\portable"

echo [Portable Packaging 1/3] Cleaning old files...
if exist "%PORTABLE_DIR%" (
    rmdir /s /q "%PORTABLE_DIR%" 2>nul
)
mkdir "%PORTABLE_DIR%"
echo Cleanup completed
echo.

echo [Portable Packaging 2/3] Deploying Qt dependencies...
if not exist "%BUILD_DIR%\PersonalDateAssisant.exe" (
    echo [ERROR] Executable not found at: %BUILD_DIR%\PersonalDateAssisant.exe
    echo Please run package.bat for full build and packaging first
    echo.
    pause
    exit /b 1
)

call "%SCRIPT_DIR%deploy.bat"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Qt dependency deployment failed!
    pause
    exit /b 1
)
echo Deployment completed
echo.

echo [Portable Packaging 3/3] Creating portable package...
echo.

xcopy /e /y "%DEPLOY_DIR%\*" "%PORTABLE_DIR%\" >nul

echo Creating README file...
(
echo DataAssistant Portable Version
echo ========================================
echo.
echo Usage:
echo 1. Run PersonalDateAssisant.exe to start the program
echo 2. Program will create data and config files in current directory
echo 3. Portable version data is stored in:
echo    - Windows XP: %%APPDATA%%\DataAssistant
echo    - Vista+: %%LOCALAPPDATA%%\DataAssistant
echo.
echo Transfer instructions:
echo 1. Copy entire portable folder to another location or computer
echo 2. Make sure target computer runs Windows 10 or higher
echo 3. Run PersonalDateAssisant.exe directly
echo.
echo Notes:
echo 1. First run may require administrator privileges
echo 2. Some antivirus software may flag this - add to trust list
echo 3. Config and data are stored in user directory
echo.
echo Version: 0.1
echo Build date: %date% %time%
echo ========================================
) > "%PORTABLE_DIR%\README.txt"

if exist "%PORTABLE_DIR%\PersonalDateAssisant.exe" (
    echo Portable packaging completed
) else (
    echo [ERROR] Portable packaging failed!
    pause
    exit /b 1
)
echo.

echo ========================================
echo Portable packaging completed successfully!
echo ========================================
echo.
echo Generated files:
echo 1. Portable directory: %PORTABLE_DIR%
echo 2. Main executable: %PORTABLE_DIR%\PersonalDateAssisant.exe
echo 3. README: %PORTABLE_DIR%\README.txt
echo.
echo Portable version features:
echo * No installation required, just run
echo * Suitable for USB drives
echo * Can be used on multiple computers
echo * Config files stored in user directory, no system impact
echo.
echo Full ReleaseTemp structure:
echo ReleaseTemp/
echo   - build/      [Compiled executable]
echo   - deploy/     [Deployment package]
echo   - portable/   [Portable version]  <- You are here
echo ========================================
echo.

cd /d "%PROJECT_ROOT%"
endlocal
