@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."
where gcc >nul 2>nul
if errorlevel 1 (
  echo gcc not found. Use MinGW64 cmd.exe or add gcc to PATH.
  exit /b 1
)
if not exist build mkdir build

echo building tau_sync_sieve_full_validation.exe
gcc -O3 -march=native -std=c11 -Wall -Wextra -DNDEBUG -fopenmp -Wl,--stack,268435456 src\tau_sync_sieve_full_validation.c -o build\tau_sync_sieve_full_validation.exe
if errorlevel 1 exit /b 1

echo building tau_m8388608_benchmark.exe
gcc -O3 -march=native -std=c11 -Wall -Wextra -DNDEBUG src\tau_m8388608_benchmark.c -o build\tau_m8388608_benchmark.exe -lm
if errorlevel 1 exit /b 1

echo building actual_values_exporter.exe
gcc -O3 -march=native -std=c11 -Wall -Wextra -DNDEBUG src\actual_values_exporter.c -o build\actual_values_exporter.exe -lm
if errorlevel 1 exit /b 1

echo building check_all_rows_results.exe
gcc -O2 -std=c11 -Wall -Wextra -DNDEBUG src\check_all_rows_results.c -o build\check_all_rows_results.exe
if errorlevel 1 exit /b 1

echo build ok
