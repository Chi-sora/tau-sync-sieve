@echo off
setlocal
where g++ >nul 2>nul
if errorlevel 1 (
  echo error: g++ not found. Install MinGW-w64 and add it to PATH.
  exit /b 1
)

g++ -std=c++17 -O3 -DNDEBUG -Wall -Wextra -pthread ^
  main.cpp tau_binary.cpp tau_index.cpp tau_query.cpp tau_twin.cpp ^
  -o tausync.exe

if errorlevel 1 (
  echo build failed
  exit /b 1
)

echo build ok: tausync.exe
endlocal
