@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."
if not exist build\tau_m8388608_benchmark.exe call scripts\build_windows.bat
if errorlevel 1 exit /b 1
if not exist results mkdir results
copy /Y data\certificates\tau_only_classification_rows.csv tau_only_classification_rows.csv >nul
copy /Y data\certificates\twin_companion_certificate.csv twin_companion_certificate.csv >nul

echo running M=8388608 all-row comparison...
build\tau_m8388608_benchmark.exe --M 8388608 --all-rows results\m8388608_all_rows_comparison.csv --summary results\m8388608_all_rows_summary.csv
set EC=%ERRORLEVEL%
echo exit_code=%EC%
exit /b %EC%
