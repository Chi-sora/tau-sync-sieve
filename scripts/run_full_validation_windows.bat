@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."
if not exist build\tau_sync_sieve_full_validation.exe call scripts\build_windows.bat
if errorlevel 1 exit /b 1
set THREADS=16
set IN=data\inputs\base_external_validation_inputs.csv
set OUT=results\tau_sync_sieve_full_results.csv
if exist "%OUT%" del /f /q "%OUT%"
echo running full validation with %THREADS% threads...
build\tau_sync_sieve_full_validation.exe "%IN%" "%OUT%" %THREADS% full
set EC=%ERRORLEVEL%
echo exit_code=%EC%
exit /b %EC%
