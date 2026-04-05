@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo DataAssistant Qt Dependency Deployment v1.0
echo ========================================
echo.

set "PROJECT_NAME=PersonalDateAssisant"
set "BUILD_DIR=%~dp0..\ReleaseTemp\build"
set "DEPLOY_DIR=%~dp0..\ReleaseTemp\deploy"

if not exist "%BUILD_DIR%\%PROJECT_NAME%.exe" (
    echo [ERROR] Executable not found at: %BUILD_DIR%\%PROJECT_NAME%.exe
    echo Please run build.bat first to compile the project
    echo.
    pause
    exit /b 1
)

echo [Step 1/3] Creating deployment directory...
if exist "%DEPLOY_DIR%" (
    rmdir /s /q "%DEPLOY_DIR%" 2>nul
)
mkdir "%DEPLOY_DIR%"
echo Deployment directory created: %DEPLOY_DIR%
echo.

echo [Step 2/3] Copying executable...
copy "%BUILD_DIR%\%PROJECT_NAME%.exe" "%DEPLOY_DIR%\" >nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to copy executable!
    pause
    exit /b 1
)
echo Executable copied
echo.

echo [Step 3/3] Deploying Qt dependencies...
where windeployqt >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] windeployqt not found!
    echo Please make sure Qt is installed and added to system PATH
    echo.
    echo Manual solution:
    echo 1. Open Qt Command Prompt
    echo 2. Run: windeployqt "%DEPLOY_DIR%\%PROJECT_NAME%.exe"
    pause
    exit /b 1
)

cd /d "%DEPLOY_DIR%"
windeployqt "%PROJECT_NAME%.exe" --no-translations --no-system-d3dcompiler --no-opengl-sw

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [WARNING] windeployqt reported issues, but continuing packaging
    echo Please check output above for serious errors
)
echo Qt dependencies deployed
echo.

echo [Extra] Copying QtAwesome font resources...
set "FONT_SOURCE=%~dp0..\lib\QtAwesome\QtAwesome\fonts"
if exist "%FONT_SOURCE%" (
    if not exist "%DEPLOY_DIR%\fonts" mkdir "%DEPLOY_DIR%\fonts"
    xcopy /e /y "%FONT_SOURCE%\*" "%DEPLOY_DIR%\fonts\" >nul
    echo Font resources copied
) else (
    echo [INFO] QtAwesome fonts not found, skipping
)
echo.

echo ========================================
echo Qt dependency deployment completed!
echo Deployment directory: %DEPLOY_DIR%
echo ========================================
echo.

cd /d "%~dp0.."
endlocal
