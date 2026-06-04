@echo off
setlocal
if not exist tau_gen_candidates_block.exe call build.bat
if errorlevel 1 exit /b 1

tau_gen_candidates_block.exe --steps 10000000000 --filter-primes 10000 --block-size 1048576 --emit out\candidates_block.csv --checkpoint checkpoint\block_state.txt --log logs\block.log --progress-every-blocks 1

echo.
echo [done] see logs\block.log
pause
endlocal
