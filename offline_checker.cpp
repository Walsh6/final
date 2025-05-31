
#include "offline_checker.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

OfflineChecker::OfflineChecker() : addresses_loaded(false) {}

bool OfflineChecker::load_addresses_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open address file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int btc_count = 0, evm_count = 0;
    
    std::cout << "Loading addresses from " << filename << "..." << std::endl;
    
    while (std::getline(file, line)) {
        // Remove whitespace and convert to lowercase for consistency
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        
        // Convert to lowercase for EVM addresses
        std::string lower_line = line;
        std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
        
        // Detect address type
        if (line.length() >= 26 && line.length() <= 35 && (line[0] == '1' || line[0] == '3' || (line[0] == 'b' && line[1] == 'c' && line[2] == '1'))) {
            // Bitcoin address (Legacy: 1..., P2SH: 3..., Bech32: bc1...)
            btc_addresses.insert(line);
            btc_count++;
        }
        else if (line.length() == 42 && line.substr(0, 2) == "0x") {
            // Ethereum/EVM address (0x...)
            evm_addresses.insert(lower_line);
            evm_count++;
        }
        else if (line.length() == 40) {
            // Ethereum address without 0x prefix
            evm_addresses.insert("0x" + lower_line);
            evm_count++;
        }
    }
    
    file.close();
    addresses_loaded = true;
    
    std::cout << "Loaded " << btc_count << " Bitcoin addresses and " 
              << evm_count << " EVM addresses" << std::endl;
    
    return btc_count > 0 || evm_count > 0;
}

bool OfflineChecker::check_btc_address(const std::string& address) {
    return btc_addresses.find(address) != btc_addresses.end();
}

bool OfflineChecker::check_evm_address(const std::string& address) {
    std::string lower_addr = address;
    std::transform(lower_addr.begin(), lower_addr.end(), lower_addr.begin(), ::tolower);
    return evm_addresses.find(lower_addr) != evm_addresses.end();
}

// Simple Keccak-256 implementation for Ethereum address generation
void keccak256(const unsigned char* input, size_t len, unsigned char* output) {
    // Simplified Keccak-256 for demonstration
    // In production, use a proper Keccak implementation
    for (int i = 0; i < 32; i++) {
        output[i] = 0;
        for (size_t j = 0; j < len; j++) {
            output[i] ^= input[j] * (i + 1) * (j + 1);
        }
        output[i] ^= (i * 137); // Add entropy
    }
}

std::string private_key_to_eth_address(const std::string& private_key_hex) {
    // Convert hex private key to bytes
    std::vector<unsigned char> private_key(32);
    for (int i = 0; i < 32 && i * 2 < private_key_hex.length(); i++) {
        std::string hex_byte = private_key_hex.substr(i * 2, 2);
        private_key[i] = (unsigned char)strtol(hex_byte.c_str(), nullptr, 16);
    }
    
    // Generate public key from private key (simplified)
    unsigned char public_key[64];
    for (int i = 0; i < 32; i++) {
        public_key[i] = private_key[i] ^ 0x04; // Simplified public key derivation
        public_key[i + 32] = private_key[i] ^ 0x08;
    }
    
    // Keccak-256 hash of public key
    unsigned char hash[32];
    keccak256(public_key, 64, hash);
    
    // Take last 20 bytes and format as hex address
    std::ostringstream address;
    address << "0x";
    for (int i = 12; i < 32; i++) {
        address << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    }
    
    return address.str();
}

size_t OfflineChecker::get_btc_count() const {
    return btc_addresses.size();
}

size_t OfflineChecker::get_evm_count() const {
    return evm_addresses.size();
}

bool OfflineChecker::load_btc_addresses(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open BTC database file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int count = 0;
    
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (line.empty() || line[0] == '#') continue;
        
        if (line.length() >= 26 && line.length() <= 35 && 
            (line[0] == '1' || line[0] == '3' || (line[0] == 'b' && line[1] == 'c' && line[2] == '1'))) {
            btc_addresses.insert(line);
            count++;
        }
    }
    
    file.close();
    addresses_loaded = true;
    std::cout << "Loaded " << count << " BTC addresses from " << filename << std::endl;
    return count > 0;
}

bool OfflineChecker::load_evm_addresses(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open EVM database file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int count = 0;
    
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (line.empty() || line[0] == '#') continue;
        
        std::string lower_line = line;
        std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
        
        if (line.length() == 42 && line.substr(0, 2) == "0x") {
            evm_addresses.insert(lower_line);
            count++;
        } else if (line.length() == 40) {
            evm_addresses.insert("0x" + lower_line);
            count++;
        }
    }
    
    file.close();
    std::cout << "Loaded " << count << " EVM addresses from " << filename << std::endl;
    return count > 0;
}
