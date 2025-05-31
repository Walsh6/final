
# 🪟 CUDA Bitcoin Wallet Generator - Windows Edition

**RTX 3060 Optimized for Windows 10/11**

## 🚀 Quick Setup

### Prerequisites
- **Windows 10/11 (64-bit)**
- **NVIDIA RTX 3060** with latest drivers
- **8GB+ RAM** (16GB recommended)

### Automatic Installation
```cmd
# Run as Administrator
install_windows.bat
```

### Manual Installation
1. **Install Visual Studio 2022 Build Tools**
   - Download from Microsoft
   - Select "C++ build tools" workload

2. **Install CUDA Toolkit 12.3**
   - Download from NVIDIA Developer site
   - Choose Windows x86_64 installer

3. **Install Git** (if not present)
   - Download from git-scm.com

## 🔨 Building

### Option 1: Batch Script (Recommended)
```cmd
build_windows.bat
```

### Option 2: Developer Command Prompt
```cmd
# Open "Developer Command Prompt for VS 2022"
nmake -f Makefile.win
```

### Option 3: Manual Build
```cmd
nvcc -std=c++14 -O3 -arch=sm_86 ^
     -I"vcpkg/installed/x64-windows/include" ^
     -L"vcpkg/installed/x64-windows/lib" ^
     main.cpp gpu_accelerated.cu ^
     -lcurl -lssl -lcrypto -lcurand ^
     -o walletgen.exe
```

## 🎮 Running

```cmd
walletgen.exe
```

## 📊 Windows Performance

### RTX 3060 on Windows
- **Wallet Generation**: 45,000-90,000/sec
- **Memory Usage**: 2-4GB GPU RAM
- **CPU Usage**: <5% (GPU-only processing)
- **Power Usage**: ~170W

## 🔧 Troubleshooting

### CUDA Not Found
```cmd
# Check CUDA installation
nvcc --version
nvidia-smi

# Add to PATH if needed
set PATH=%PATH%;C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.3\bin
```

### Build Errors
```cmd
# Use Developer Command Prompt
"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

### Performance Issues
```cmd
# Check GPU status
nvidia-smi
# Close other applications using GPU
```

## 🛡️ Windows Security

- **Windows Defender**: May flag as potential threat (false positive)
- **Firewall**: Allow network access for balance checking
- **Antivirus**: Add exception for wallet generator folder

## 📁 File Locations

```
Project/
├── walletgen.exe          # Main executable
├── bip39-words.txt        # Word list
├── found_wallets.txt      # Results (created when found)
├── build_windows.bat      # Build script
└── vcpkg/                 # Dependencies
```

## 🎯 Usage Tips

1. **Run from SSD** for better I/O performance
2. **Close browsers/games** to free GPU memory
3. **Monitor temperatures** with MSI Afterburner
4. **Use stable overclock** for maximum performance

## ⚡ Windows Optimizations

- **Power Plan**: High Performance
- **GPU Scheduler**: Hardware-accelerated (Windows 10 2004+)
- **Game Mode**: Disabled
- **Background Apps**: Minimize

Ready to generate wallets at maximum speed on Windows! 🚀
