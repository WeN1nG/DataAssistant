@echo off
echo ========================================
echo IMAP Connection Diagnostic Script
echo ========================================
echo.

echo [1/5] Testing TCP port connectivity...
echo.
powershell -Command "Test-NetConnection -ComputerName imap.qq.com -Port 993"

echo.
echo ========================================
echo [2/5] Checking for listening connections on port 993...
echo.
netstat -an | findstr ":993"

echo.
echo ========================================
echo [3/5] Checking system proxy settings...
echo.
powershell -Command "Get-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings' | Select-Object ProxyEnable, ProxyServer"

echo.
echo ========================================
echo [4/5] Checking environment variables for proxy...
echo.
echo HTTP_PROXY: %HTTP_PROXY%
echo HTTPS_PROXY: %HTTPS_PROXY%
echo NO_PROXY: %NO_PROXY%

echo.
echo ========================================
echo [5/5] Testing with OpenSSL (if available)...
echo.
where openssl >nul 2>&1
if %errorlevel% equ 0 (
    echo OpenSSL found, testing connection...
    echo Type: QUIT to exit
    echo.
    echo A1 CAPABILITY > temp_imap.txt
    type temp_imap.txt | openssl s_client -connect imap.qq.com:993 -brief
    del temp_imap.txt
) else (
    echo OpenSSL not found in PATH
    echo.
    echo You can manually test with:
    echo   telnet imap.qq.com 993
    echo Or download OpenSSL from: https://slproweb.com/products/Win32OpenSSL.html
)

echo.
echo ========================================
echo Diagnostic completed!
echo ========================================
echo.
echo NEXT STEPS:
echo 1. If Test-NetConnection shows TcpTestSucceeded: True, port is reachable
echo 2. If port is reachable but application fails, try disabling firewall temporarily
echo 3. Check Windows Security Center for blocking notifications
echo 4. Try running SimpleTest.cpp for simplified connection test
echo.
pause
