@echo off
setlocal

echo Building tau_index.exe with MinGW-w64...
gcc -O3 -std=c99 -Wall -Wextra -o tau_index.exe main.c

if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo Build OK: tau_index.exe
echo.
echo Examples:
echo   tau_index.exe build 60000000 index.bin 8 1024
echo   tau_index.exe query index.bin 10000
echo   tau_index.exe twins index.bin 1 60000000 10
