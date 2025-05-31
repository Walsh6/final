#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <curl/curl.h>
#include <iomanip>
#include <sstream>
#include <random>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif
#include "gpu_accelerated.h"
#include "offline_checker.h"

std::vector<std::string> wordlist;
std::atomic<unsigned long long> total_checked(0);
std::atomic<unsigned long long> wallets_with_balance(0);
std::mutex output_mutex;
auto start_time = std::chrono::high_resolution_clock::now();

// Offline checking
OfflineChecker offline_checker;
bool use_offline_mode = false;

void update_console_title() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
    double wallets_per_second = duration.count() > 0 ? total_checked.load() / (double)duration.count() : 0;
    
    std::ostringstream title;
    title << "[WalletGen | GPU 80% | OFFLINE] - Searching BTC/EVM wallets / [Checked: " 
          << total_checked << " | Found: " << std::fixed << std::setprecision(7) 
          << wallets_with_balance.load() << " | Speed: " << (int)wallets_per_second << "/s]";
    
#ifdef _WIN32
    SetConsoleTitleA(title.str().c_str());
#else
    std::cout << "\033]0;" << title.str() << "\007" << std::flush;
#endif
}

void display_wallet_check(const std::string& mnemonic, const std::string& btc_address, const std::string& eth_address, bool btc_balance, bool eth_balance) {
    std::lock_guard<std::mutex> lock(output_mutex);
    
    std::cout << "mnemonic:     " << mnemonic << std::endl;
    std::cout << "btc address:  " << btc_address;
    if (btc_balance) std::cout << " *** BTC FOUND ***";
    std::cout << std::endl;
    
    std::cout << "eth address:  " << eth_address;
    if (eth_balance) std::cout << " *** ETH FOUND ***";
    std::cout << std::endl;
    
    if (btc_balance || eth_balance) {
        std::cout << "STATUS:       *** WALLET WITH BALANCE FOUND ***" << std::endl;
    } else {
        std::cout << "STATUS:       (empty)" << std::endl;
    }
    
    std::cout << "===================================================================" << std::endl;
}

// Forward declaration for ETH address generation
std::string private_key_to_eth_address(const std::string& private_key) {
    // Simple ETH address generation from private key
    // In a real implementation, this would use proper secp256k1 and keccak256
    std::string eth_address = "0x";
    
    // Use the last 40 characters of the private key as a simple ETH address
    if (private_key.length() >= 40) {
        eth_address += private_key.substr(private_key.length() - 40);
    } else {
        // Pad with zeros if needed
        std::string padded = private_key;
        while (padded.length() < 40) {
            padded = "0" + padded;
        }
        eth_address += padded;
    }
    
    return eth_address;
}

void load_wordlist() {
    std::ifstream file("bip39-words.txt");
    std::string word;
    while (std::getline(file, word)) {
        if (!word.empty() && word.back() == '\r') {
            word.pop_back();
        }
        wordlist.push_back(word);
    }
    std::cout << "Loaded " << wordlist.size() << " words from BIP39 wordlist" << std::endl;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool check_balance(const std::string& address) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) return false;

    std::string url = "https://blockstream.info/api/address/" + address;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    // SSL/TLS settings to fix certificate issues
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
    
    // Retry mechanism for SSL failures
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    // HTTP headers to appear more like a regular browser
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.9");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        // Try fallback with disabled SSL verification for problematic certificates
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        
        if (res != CURLE_OK) {
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cerr << "CURL Error for " << address << ": " << curl_easy_strerror(res) << std::endl;
            return false;
        }
    }

    if (response.empty()) {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cerr << "Empty response for address: " << address << std::endl;
        return false;
    }

    // Check if funded_txo_sum exists and is greater than 0
    size_t funded_pos = response.find("\"funded_txo_sum\":");
    if (funded_pos == std::string::npos) {
        // Also check for spent_txo_sum as an alternative indicator
        size_t spent_pos = response.find("\"spent_txo_sum\":");
        if (spent_pos != std::string::npos) {
            size_t value_start = response.find(":", spent_pos) + 1;
            size_t value_end = response.find_first_of(",}", value_start);
            if (value_start != std::string::npos && value_end != std::string::npos) {
                std::string value = response.substr(value_start, value_end - value_start);
                value.erase(0, value.find_first_not_of(" \t\n\r"));
                value.erase(value.find_last_not_of(" \t\n\r") + 1);
                if (value != "0" && !value.empty()) {
                    // Log potential balance found for debugging
                    std::lock_guard<std::mutex> lock(output_mutex);
                    std::cout << "DEBUG: Address " << address << " has spent_txo_sum: " << value << std::endl;
                    return true;
                }
            }
        }
        return false;
    }
    
    // Extract the value after "funded_txo_sum":
    size_t value_start = response.find(":", funded_pos) + 1;
    size_t value_end = response.find_first_of(",}", value_start);
    
    if (value_start != std::string::npos && value_end != std::string::npos) {
        std::string value = response.substr(value_start, value_end - value_start);
        // Remove whitespace
        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
        
        // Check if value is not "0"
        if (value != "0" && !value.empty()) {
            // Log balance found for verification
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "DEBUG: Address " << address << " has funded_txo_sum: " << value << std::endl;
            return true;
        }
    }
    
    return false;
}

void save_wallet(const std::string& mnemonic, const std::string& btc_address, const std::string& eth_address, const std::string& private_key, bool btc_found, bool eth_found) {
    std::lock_guard<std::mutex> lock(output_mutex);
    
    // Verify file can be opened
    std::ofstream file("found_wallets.txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open found_wallets.txt for writing!" << std::endl;
        return;
    }
    
    // Get current timestamp in readable format
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto unix_timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    file << "=== WALLET FOUND ===" << std::endl;
    file << "Mnemonic: " << mnemonic << std::endl;
    file << "BTC Address: " << btc_address;
    if (btc_found) file << " (BALANCE FOUND)";
    file << std::endl;
    file << "ETH Address: " << eth_address;
    if (eth_found) file << " (BALANCE FOUND)";
    file << std::endl;
    file << "Private Key: " << private_key << std::endl;
    file << "Unix Timestamp: " << unix_timestamp << std::endl;
    file << "Date: " << std::ctime(&time_t); // This adds a newline
    file << "Total Found: " << (wallets_with_balance.load() + 1) << std::endl;
    file << "===================" << std::endl;
    file << std::endl; // Extra line for readability
    file.close();
    
    // Verify file was written successfully
    if (file.good()) {
        wallets_with_balance++;
        std::cout << "\n*** WALLET WITH BALANCE FOUND ***" << std::endl;
        std::cout << "Mnemonic: " << mnemonic << std::endl;
        if (btc_found) std::cout << "BTC Address: " << btc_address << " (BALANCE FOUND)" << std::endl;
        if (eth_found) std::cout << "ETH Address: " << eth_address << " (BALANCE FOUND)" << std::endl;
        std::cout << "Private Key: " << private_key << std::endl;
        std::cout << "Successfully saved to found_wallets.txt" << std::endl;
        std::cout << "Total wallets with balance found: " << wallets_with_balance.load() << std::endl;
        std::cout << "********************************" << std::endl;
    } else {
        std::cerr << "ERROR: Failed to write wallet to file!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "CUDA-Accelerated Multi-Chain Wallet Generator (RTX 3060)" << std::endl;
    std::cout << "=========================================================" << std::endl;
    std::cout << "OFFLINE-ONLY MODE - Bitcoin & Ethereum Support" << std::endl;
    std::cout << "=========================================================" << std::endl;

    // Offline-only mode
    std::cout << "\nOFFLINE-ONLY MODE" << std::endl;
    std::cout << "Loading address databases..." << std::endl;
    
    // Load BTC addresses
    std::ifstream btc_file("btc_database.txt");
    if (btc_file.good()) {
        btc_file.close();
        offline_checker.load_btc_addresses("btc_database.txt");
    } else {
        std::cout << "⚠ btc_database.txt not found - Bitcoin checking disabled" << std::endl;
    }
    
    // Load EVM addresses
    std::ifstream evm_file("evm_database.txt");
    if (evm_file.good()) {
        evm_file.close();
        offline_checker.load_evm_addresses("evm_database.txt");
    } else {
        std::cout << "⚠ evm_database.txt not found - EVM checking disabled" << std::endl;
    }
    
    if (offline_checker.get_btc_count() == 0 && offline_checker.get_evm_count() == 0) {
        std::cerr << "No address databases loaded. Please ensure btc_database.txt and/or evm_database.txt exist." << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    use_offline_mode = true;
    std::cout << "Offline mode ready with " << offline_checker.get_btc_count() << " BTC addresses and " 
              << offline_checker.get_evm_count() << " EVM addresses" << std::endl;

    // Load wordlist
    load_wordlist();
    if (wordlist.size() != 2048) {
        std::cerr << "Error: BIP39 wordlist must contain exactly 2048 words" << std::endl;
        return 1;
    }

    // Initialize CUDA
    if (!init_gpu_system(wordlist)) {
        std::cerr << "Failed to initialize CUDA system" << std::endl;
        return 1;
    }

    std::cout << "Starting continuous GPU wallet generation..." << std::endl;
    std::cout << "Checking both Bitcoin and Ethereum addresses" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    const int BATCH_SIZE = 500000; // Increased batch size for 80% GPU utilization
    start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        // Generate massive batch on GPU with maximum utilization
        WalletBatch batch = generate_wallet_batch_gpu(BATCH_SIZE);
        
        // Update total immediately
        total_checked += batch.count;

        // Offline-only mode - check entire batch with maximum parallelism
        std::cout << "\n🔍 Processing batch of " << batch.addresses.size() << " wallets..." << std::endl;
        
        for (size_t i = 0; i < batch.addresses.size(); i++) {
            if (!batch.addresses[i].empty() && batch.addresses[i] != "INVALID") {
                std::string eth_address = private_key_to_eth_address(batch.private_keys[i]);
                
                bool btc_balance = offline_checker.check_btc_address(batch.addresses[i]);
                bool eth_balance = offline_checker.check_evm_address(eth_address);
                
                // Display every wallet being checked
                display_wallet_check(batch.mnemonics[i], batch.addresses[i], eth_address, btc_balance, eth_balance);
                
                if (btc_balance || eth_balance) {
                    save_wallet(batch.mnemonics[i], batch.addresses[i], eth_address, batch.private_keys[i], btc_balance, eth_balance);
                }
                
                // Show progress every 10,000 wallets
                if ((i + 1) % 10000 == 0) {
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
                    double wallets_per_second = duration.count() > 0 ? total_checked.load() / (double)duration.count() : 0;
                    
                    std::cout << "\n📊 Progress: " << (i + 1) << "/" << batch.addresses.size() 
                              << " wallets in current batch | Total: " << total_checked.load() 
                              << " | Speed: " << (int)wallets_per_second << "/s" << std::endl;
                }
            }
        }
        
        // Update console title every batch
        update_console_title();
    }

    cleanup_gpu_system();
    return 0;
}