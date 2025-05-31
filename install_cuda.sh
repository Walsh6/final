
#!/bin/bash

echo "🔧 CUDA Installation for RTX 3060"
echo "================================="

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "❌ Please don't run this script as root"
   exit 1
fi

# Check for NVIDIA GPU
echo "🔍 Checking for NVIDIA GPU..."
if ! command -v nvidia-smi &> /dev/null; then
    echo "📦 Installing NVIDIA drivers..."
    sudo apt update
    sudo apt install -y nvidia-driver-525 nvidia-dkms-525
    echo "✓ NVIDIA drivers installed."
    echo "⚠️  Please reboot your system and run this script again."
    exit 1
fi

echo "✓ NVIDIA GPU detected:"
nvidia-smi --query-gpu=name,driver_version --format=csv,noheader

# Check if CUDA is already installed
if command -v nvcc &> /dev/null; then
    echo "✓ CUDA already installed:"
    nvcc --version
    echo "✓ Ready to build GPU-accelerated wallet generator!"
    exit 0
fi

echo "📦 Installing CUDA Toolkit 12.3..."

# Create temp directory
TEMP_DIR="/tmp/cuda_install"
mkdir -p $TEMP_DIR
cd $TEMP_DIR

# Download CUDA installer
CUDA_URL="https://developer.download.nvidia.com/compute/cuda/12.3.0/local_installers/cuda_12.3.0_545.23.06_linux.run"
echo "⬇️  Downloading CUDA installer..."
wget -q --show-progress $CUDA_URL

# Make installer executable
chmod +x cuda_12.3.0_545.23.06_linux.run

# Install CUDA (silent mode, toolkit only)
echo "🔧 Installing CUDA Toolkit..."
sudo ./cuda_12.3.0_545.23.06_linux.run --silent --toolkit --no-opengl-libs

# Add CUDA to PATH
echo "🔧 Configuring environment..."
echo '# CUDA Environment' >> ~/.bashrc
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc

# Apply immediately
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Cleanup
cd ~
rm -rf $TEMP_DIR

echo ""
echo "✅ CUDA installation completed!"
echo "🎯 Configured for RTX 3060 (Compute Capability 8.6)"
echo ""
echo "Verification:"
nvcc --version 2>/dev/null || echo "Run 'source ~/.bashrc' to load CUDA environment"
echo ""
echo "Next steps:"
echo "1. Run 'source ~/.bashrc' or restart terminal"
echo "2. Run './build_gpu.sh' to build the wallet generator"
echo ""
echo "Your RTX 3060 is ready for maximum performance! 🚀"
