@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."
if not exist build\actual_values_exporter.exe call scripts\build_windows.bat
if errorlevel 1 exit /b 1
copy /Y data\certificates\tau_only_classification_rows.csv tau_only_classification_rows.csv >nul
copy /Y data\certificates\twin_companion_certificate.csv twin_companion_certificate.csv >nul
build\actual_values_exporter.exe
if not exist results mkdir results
move /Y m8388608_actual_values_*.csv results\ >nul 2>nul
echo actual value CSV files are in results\
