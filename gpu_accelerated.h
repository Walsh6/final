
#ifndef GPU_ACCELERATED_H
#define GPU_ACCELERATED_H

#include <vector>
#include <string>

// Windows compatibility
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

struct WalletBatch {
    std::vector<std::string> mnemonics;
    std::vector<std::string> addresses;
    std::vector<std::string> private_keys;
    int count;
};

// Core GPU functions
bool init_gpu_system(const std::vector<std::string>& wordlist);
WalletBatch generate_wallet_batch_gpu(int batch_size);
void cleanup_gpu_system();

// GPU performance monitoring
void get_gpu_stats();
bool check_gpu_memory();

#endif
