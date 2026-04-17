@echo off
REM Quick Test Runner Script
REM Runs the email service tests and displays results

echo ========================================
echo Running Email Service Tests
echo ========================================
echo.

if not exist "build-test\EmailServiceTest.exe" (
    echo [ERROR] Test executable not found!
    echo Please run build_tests.bat first.
    pause
    exit /b 1
)

cd build-test

echo [INFO] Running tests...
EmailServiceTest.exe

echo.
echo ========================================
echo Test execution completed
echo ========================================
echo.

if exist "test_report.txt" (
    echo [INFO] Test report saved to: test_report.txt
)

if exist "test_report.html" (
    echo [INFO] HTML report saved to: test_report.html
)

pause
