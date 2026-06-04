@echo off
setlocal
echo [build] tau_gen_candidates_block

where gcc >nul 2>nul
if errorlevel 1 (
  echo [error] gcc not found.
  pause
  exit /b 1
)

set CFLAGS=-O3 -std=c11 -Wall -Wextra -march=native

if not exist out mkdir out
if not exist logs mkdir logs
if not exist checkpoint mkdir checkpoint

gcc %CFLAGS% -o tau_gen_candidates_block.exe src\tau_gen_candidates_block.c
if errorlevel 1 (
  echo [error] build failed.
  pause
  exit /b 1
)

echo [ok] built tau_gen_candidates_block.exe
pause
endlocal
