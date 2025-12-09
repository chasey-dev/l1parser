#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>

static const char* L1_DAT_PATH = "/etc/wireless/l1profile.dat";

// Represents a single Radio/Band configuration (e.g., MT7981_1_1)
struct L1Entry {
    std::string index_name; // e.g. "MT7981"
    size_t main_idx;        // Chipset index (1st, 2nd of its kind)
    size_t sub_idx;         // Band index (1, 2...)
    std::unordered_map<std::string, std::string> props;
};

// Represents a raw block in the config file, used for sequential indexing
struct RawBlock {
    size_t raw_index;
    std::vector<std::string> main_ifnames;
};

class L1Parser {
public:
    L1Parser();
    bool load(const std::string path);

    // Core logic getters
    std::optional<std::string> get_prop(const std::string& dev, const std::string& key) const;
    std::vector<std::string> list_devs() const;
    std::optional<std::string> if2zone(const std::string& ifname) const;
    std::optional<std::string> if2dat(const std::string& ifname) const;
    std::optional<std::string> if2dbdcidx(const std::string& ifname) const;
    std::vector<std::string> zone2if(const std::string& zone) const;
    std::optional<std::string> idx2if(int idx) const;

    // Additional helpers
    const std::unordered_map<std::string, L1Entry>& get_if_map() const { return if_map_; }

private:
    std::unordered_map<std::string, L1Entry> dev_map_; // Map by Device ID: "MT7981_1_1"
    std::unordered_map<std::string, L1Entry> if_map_;  // Map by Interface: "ra0", "apcli0"
    std::vector<RawBlock> raw_blocks_;
    std::vector<std::string> ordered_dev_keys_;

    // Map RawIndex -> { PropertyKey -> Value }
    using RawDataMap = std::map<size_t, std::unordered_map<std::string, std::string>>;

    RawDataMap parse_raw_config(const std::string& path);

    void process_block(size_t raw_idx, 
                       const std::unordered_map<std::string, std::string>& props,
                       std::unordered_map<std::string, size_t>& chipset_counter);

    void create_and_map_entry(const std::string& chip_name,
                              size_t main_idx,
                              size_t sub_idx,
                              size_t raw_idx,
                              size_t band_index, // 0-based index in main_ifnames
                              const std::string& current_main_if,
                              const std::unordered_map<std::string, std::string>& props);

    static std::optional<std::pair<size_t, std::string>> parse_index_key(const std::string& key);          
};