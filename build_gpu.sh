
#!/bin/bash

echo "🚀 CUDA-Accelerated Bitcoin Wallet Generator Builder"
echo "=================================================="
echo "GPU-ONLY MODE - Maximum Performance for RTX 3060"
echo "=================================================="

# Check for NVIDIA GPU
if ! command -v nvidia-smi &> /dev/null; then
    echo "❌ NVIDIA drivers not found!"
    echo "Please install NVIDIA drivers first:"
    echo "sudo apt install nvidia-driver-470"
    echo "Then reboot and run this script again."
    exit 1
fi

echo "✓ NVIDIA GPU detected:"
nvidia-smi --query-gpu=name,memory.total --format=csv,noheader

# Check for CUDA
if ! command -v nvcc &> /dev/null; then
    echo ""
    echo "❌ CUDA Toolkit not found!"
    echo "Installing CUDA automatically..."
    chmod +x install_cuda.sh
    ./install_cuda.sh
    echo ""
    echo "Please restart your terminal and run this script again."
    exit 1
fi

echo "✓ CUDA Toolkit found:"
nvcc --version | grep release

# Install build dependencies
echo ""
echo "📦 Installing dependencies..."
sudo apt update > /dev/null 2>&1
sudo apt install -y build-essential libcurl4-openssl-dev > /dev/null 2>&1

# Build the GPU-accelerated version
echo ""
echo "🔨 Building GPU-accelerated wallet generator..."
echo "Optimizing for RTX 3060 architecture (sm_86)..."

make clean > /dev/null 2>&1
make

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 Build successful!"
    echo "=================================="
    echo "GPU-accelerated wallet generator is ready!"
    echo ""
    echo "Performance specs:"
    echo "- Architecture: CUDA sm_86 (RTX 3060)"
    echo "- Processing: Pure GPU acceleration"
    echo "- Expected rate: 50,000+ wallets/second"
    echo "- Memory usage: ~2-4GB GPU RAM"
    echo ""
    echo "Usage:"
    echo "  ./walletgen"
    echo ""
    echo "Found wallets will be saved to 'found_wallets.txt'"
    echo "Press Ctrl+C to stop the generator"
    echo ""
    
    # Quick GPU memory check
    echo "GPU Memory Status:"
    nvidia-smi --query-gpu=memory.used,memory.free,memory.total --format=csv,noheader,nounits | awk '{print "Used: "$1"MB, Free: "$2"MB, Total: "$3"MB"}'
    echo ""
    echo "Ready to launch! 🚀"
else
    echo ""
    echo "❌ Build failed!"
    echo "Check the error messages above and ensure:"
    echo "1. CUDA Toolkit is properly installed"
    echo "2. NVIDIA drivers are up to date"
    echo "3. RTX 3060 is detected by nvidia-smi"
    exit 1
fi
