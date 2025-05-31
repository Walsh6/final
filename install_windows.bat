
@echo off
echo ========================================
echo Windows Setup for CUDA Wallet Generator
echo ========================================

REM Check for admin privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo This script requires administrator privileges
    echo Please run as administrator
    pause
    exit /b 1
)

echo Installing required components...

REM Check for Chocolatey
where choco >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing Chocolatey package manager...
    powershell -Command "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
)

REM Install Git if not present
where git >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing Git...
    choco install git -y
)

REM Install Visual Studio Build Tools
echo Installing Visual Studio Build Tools 2022...
choco install visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" -y

REM Download and install CUDA
echo.
echo CUDA Toolkit Installation Required
echo ==================================
echo Please download and install CUDA Toolkit 12.3 from:
echo https://developer.nvidia.com/cuda-downloads
echo.
echo Select:
echo - Windows
echo - x86_64 
echo - Your Windows version
echo - exe (local)
echo.
echo After CUDA installation, restart your computer and run build_windows.bat
echo.
pause

echo Setup completed! 
echo Next steps:
echo 1. Install CUDA Toolkit 12.3 from NVIDIA
echo 2. Restart your computer
echo 3. Run build_windows.bat from Developer Command Prompt
pause
