import os

# Recreate output folder and write the batch file again after reset
os.makedirs("/mnt/data/ProcWolfCPP", exist_ok=True)

build_bat = r'''@echo off
setlocal

echo [*] Creating build directory...
if not exist build mkdir build
cd build

echo [*] Running CMake...
cmake .. || goto :error

echo [*] Building all targets...
cmake --build . --config Release || goto :error

echo [*] Build complete!
echo Output binaries are in: %cd%\Release (or current folder if not using multi-config generator)
pause
exit /b 0

:error
echo [!] Build failed.
pause
exit /b 1