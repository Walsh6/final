# GPU-Only Makefile for RTX 3060 Bitcoin Wallet Generator
NVCC = nvcc
CXX = g++
CUDA_ARCH = -arch=sm_86
CXXFLAGS = -std=c++17 -O3 -Wall
NVCCFLAGS = -std=c++17 -O3 $(CUDA_ARCH) --use_fast_math -Xcompiler -fopenmp
LDFLAGS = -lcurl -lcudart -lgomp

# Source files
CUDA_SOURCES = gpu_accelerated.cu
CPP_SOURCES = main.cpp offline_checker.cpp
HEADERS = gpu_accelerated.h offline_checker.h

# Object files
CUDA_OBJECTS = $(CUDA_SOURCES:.cu=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

# Target executable
TARGET = walletgen

# Default target - GPU only
all: check-cuda $(TARGET)

# Check for CUDA installation
check-cuda:
	@which nvcc > /dev/null || (echo "ERROR: CUDA not found. Run ./install_cuda.sh first." && exit 1)
	@echo "✓ CUDA found: $$(nvcc --version | grep release)"
	@nvidia-smi > /dev/null || (echo "ERROR: NVIDIA drivers not found" && exit 1)
	@echo "✓ NVIDIA drivers detected"

# Compile CUDA source with optimizations
gpu_accelerated.o: gpu_accelerated.cu gpu_accelerated.h
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

# Compile C++ source
main.o: main.cpp gpu_accelerated.h offline_checker.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

offline_checker.o: offline_checker.cpp offline_checker.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link everything
$(TARGET): $(CUDA_OBJECTS) $(CPP_OBJECTS)
	$(NVCC) $(NVCCFLAGS) $(CUDA_OBJECTS) $(CPP_OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@echo "✓ GPU-accelerated wallet generator built successfully!"
	@echo "✓ Optimized for RTX 3060 (sm_86 architecture)"
	@echo ""
	@echo "Usage: ./$(TARGET)"
	@echo ""

# Performance test
test-performance: $(TARGET)
	@echo "Running performance test..."
	timeout 10s ./$(TARGET) || true
	@echo "Performance test completed."

# GPU information
gpu-info:
	nvidia-smi
	nvcc --version

# Clean build files
clean:
	rm -f $(CUDA_OBJECTS) $(CPP_OBJECTS) $(TARGET)
	@echo "Build files cleaned."

# Install dependencies
install-deps:
	sudo apt update
	sudo apt install -y build-essential libcurl4-openssl-dev
	@echo "Dependencies installed."

# Install CUDA if needed
install-cuda:
	chmod +x install_cuda.sh
	./install_cuda.sh

# Memory test
test-memory:
	nvidia-smi --query-gpu=memory.total,memory.used,memory.free --format=csv

.PHONY: all clean install-deps install-cuda check-cuda gpu-info test-performance test-memory