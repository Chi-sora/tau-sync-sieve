@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."
if not exist build\check_all_rows_results.exe call scripts\build_windows.bat
if errorlevel 1 exit /b 1
build\check_all_rows_results.exe results\m8388608_all_rows_comparison.csv 8388608 results\all_rows_check_summary.csv
set EC=%ERRORLEVEL%
echo exit_code=%EC%
exit /b %EC%
