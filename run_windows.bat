
@echo off
title CUDA Bitcoin Wallet Generator - RTX 3060

echo ============================================
echo  CUDA Bitcoin Wallet Generator - Windows
echo  RTX 3060 Maximum Performance Mode
echo ============================================
echo.

REM Check if executable exists
if not exist "walletgen.exe" (
    echo walletgen.exe not found!
    echo.
    echo Building the project...
    call build_windows.bat
    echo.
)

REM Check for BIP39 wordlist
if not exist "bip39-words.txt" (
    echo ERROR: bip39-words.txt not found!
    echo Please ensure the BIP39 wordlist file is in the same directory.
    pause
    exit /b 1
)

REM Check GPU
nvidia-smi >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: nvidia-smi not found or GPU not detected
    echo Please ensure NVIDIA drivers are installed
    pause
)

echo Starting wallet generator...
echo Press Ctrl+C to stop
echo.
echo Found wallets will be saved to: found_wallets.txt
echo.

REM Run the wallet generator
walletgen.exe

echo.
echo Wallet generator stopped.
pause
