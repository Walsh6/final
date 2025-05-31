
#!/bin/bash

echo "GPU-Accelerated Bitcoin Wallet Generator"
echo "======================================="

# Check and install dependencies
if ! dpkg -l | grep -q "libssl"; then
    echo "libssl library not found. Installing..."
    sudo apt update
    sudo apt install -y libssl3
fi

if ! dpkg -l | grep -q "libcurl4"; then
    echo "libcurl4 library not found. Installing..."
    sudo apt install -y libcurl4
fi

# Check if GPU version exists
if [ -f "./walletgen" ]; then
    echo "Running GPU-accelerated version..."
    ./walletgen
elif [ -f "./walletgen_cpu" ]; then
    echo "Running CPU-only version..."
    ./walletgen_cpu
else
    echo "No compiled binary found. Building..."
    chmod +x build_gpu.sh
    ./build_gpu.sh
    
    if [ -f "./walletgen" ]; then
        echo "Starting GPU-accelerated wallet generator..."
        ./walletgen
    else
        echo "Build failed. Please check the error messages above."
    fi
fi
