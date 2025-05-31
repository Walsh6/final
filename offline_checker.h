#ifndef OFFLINE_CHECKER_H
#define OFFLINE_CHECKER_H

#include <string>
#include <unordered_set>

class OfflineChecker {
private:
    std::unordered_set<std::string> btc_addresses;
    std::unordered_set<std::string> evm_addresses;
    bool addresses_loaded;

public:
    OfflineChecker();
    bool load_addresses_from_file(const std::string& filename);
    bool load_btc_addresses(const std::string& filename);
    bool load_evm_addresses(const std::string& filename);
    bool check_btc_address(const std::string& address);
    bool check_evm_address(const std::string& address);

    size_t get_btc_count() const;
    size_t get_evm_count() const;
    bool is_loaded() const { return addresses_loaded; }
};

// Ethereum address generation function
std::string private_key_to_eth_address(const std::string& private_key_hex);

#endif