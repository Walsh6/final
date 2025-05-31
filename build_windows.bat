
@echo off
echo CUDA Bitcoin Wallet Generator - Windows Build
echo ==========================================

REM Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running with Administrator privileges - Good!
) else (
    echo WARNING: Not running as Administrator
    echo Some installations may fail
)

REM Check for Visual Studio
where cl >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Visual Studio Build Tools not found!
    echo Please install Visual Studio 2022 Build Tools
    echo Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
    pause
    exit /b 1
)

REM Check for CUDA
where nvcc >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: CUDA Toolkit not found!
    echo Please install CUDA Toolkit 12.3
    echo Download from: https://developer.nvidia.com/cuda-downloads
    pause
    exit /b 1
)

echo Found Visual Studio Build Tools
echo Found CUDA Toolkit

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %errorLevel% neq 0 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

echo Setting up development environment...

REM Build the project
echo Building CUDA-accelerated wallet generator...
nmake -f Makefile.win clean
nmake -f Makefile.win

if %errorLevel% equ 0 (
    echo.
    echo BUILD SUCCESSFUL!
    echo ================
    echo.
    echo Your GPU-accelerated wallet generator is ready!
    echo Run: walletgen.exe
    echo.
    echo Performance optimized for RTX 3060
    echo Expected rate: 50,000+ wallets/second
    echo.
) else (
    echo.
    echo BUILD FAILED!
    echo ============
    echo Check the error messages above
    echo.
    echo Common issues:
    echo 1. Missing Visual Studio Build Tools
    echo 2. Missing CUDA Toolkit
    echo 3. Missing NVIDIA drivers
    echo.
)

pause
