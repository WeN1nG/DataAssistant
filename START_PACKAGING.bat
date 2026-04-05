@echo off
chcp 65001 >nul

echo ========================================
echo DataAssistant Windows Packaging Tool
echo ========================================
echo.
echo Please select an option:
echo.
echo 1. Full Package (Recommended) [build.bat + deploy.bat + installer]
echo    Generate standard Windows installer
echo.
echo 2. Portable Package
echo    Generate portable version (no install required)
echo.
echo 3. Environment Check
echo    Verify build environment
echo.
echo 4. Security Verification
echo    Windows Defender scan
echo.
echo 5. View Documentation
echo    Open QUICKSTART guide
echo.
echo 6. Exit
echo.
echo ========================================

set /p choice=Enter option (1-6): 

if "%choice%"=="1" goto full_package
if "%choice%"=="2" goto portable
if "%choice%"=="3" goto check_env
if "%choice%"=="4" goto verify
if "%choice%"=="5" goto docs
if "%choice%"=="6" goto exit

echo.
echo [ERROR] Invalid option, please enter 1-6
echo.
pause
goto start

:full_package
echo.
echo ========================================
echo Starting full packaging process...
echo ========================================
echo.
cd /d "%~dp0scripts"
call package.bat
echo.
pause
goto start

:portable
echo.
echo ========================================
echo Starting portable packaging...
echo ========================================
echo.
cd /d "%~dp0scripts"
call portable.bat
echo.
pause
goto start

:check_env
echo.
echo ========================================
echo Checking build environment...
echo ========================================
echo.
cd /d "%~dp0scripts"
call check_env.bat
echo.
pause
goto start

:verify
echo.
echo ========================================
echo Starting security verification...
echo ========================================
echo.
cd /d "%~dp0scripts"
powershell -ExecutionPolicy Bypass -File verify_defender.ps1
echo.
pause
goto start

:docs
echo.
echo ========================================
echo Opening documentation...
echo ========================================
echo.
start "" "%~dp0QUICKSTART.md"
echo Quick start guide opened in text editor
echo.
echo You can also view:
echo - PACKAGING_GUIDE.md - Complete documentation
echo - PACKAGING_CHECKLIST.md - Checklist
echo.
pause
goto start

:exit
echo.
echo ========================================
echo Thanks for using DataAssistant Packaging Tool
echo ========================================
echo.
exit
