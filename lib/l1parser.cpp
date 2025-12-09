#include "l1parser.hpp"
#include "../utils/stringutils.hpp"
#include <fstream>
#include <algorithm>
#include <set>

// Constants for logic replication (max number of virtual interfaces)
static const size_t MAX_NUM_EXTIF = 16;
static const size_t MAX_NUM_APCLI = 1;
static const size_t MAX_NUM_WDS = 4;
static const size_t MAX_NUM_MESH = 1;

L1Parser::L1Parser() {}

bool L1Parser::load(const std::string path) {
    // Parse file into intermediate structure (sorted map ensures order)
    RawDataMap raw_data = parse_raw_config(path);
    if (raw_data.empty()) {
        // If file is empty or cannot be opened, check existence to decide return value.
        // Assuming false if file is bad.
        std::ifstream f(path);
        if (!f.good()) return false;
    }

    // Counter to track index per chipset type (e.g., 2nd MT7981 found)
    std::unordered_map<std::string, size_t> chipset_counter;

    // Iterate through Raw Data (map automatically sorts by raw_idx)
    for (const auto& [raw_idx, props] : raw_data) {
        process_block(raw_idx, props, chipset_counter);
    }

    // Sort keys to ensure consistent output for list()
    std::sort(ordered_dev_keys_.begin(), ordered_dev_keys_.end());
    return true;
}

/**
 * Parses keys like "INDEX1", "INDEX1_main_ifname", "INDEX2".
 * Returns { 1, "INDEX" } or { 1, "main_ifname" }.
 */
std::optional<std::pair<size_t, std::string>> L1Parser::parse_index_key(const std::string& key) {
    if (key.size() <= 5 || key.compare(0, 5, "INDEX") != 0) return std::nullopt;

    size_t num_start = 5;
    size_t num_end = num_start;
    // Extract numbers after "INDEX"
    while (num_end < key.size() && std::isdigit(key[num_end])) num_end++;

    if (num_end == num_start) return std::nullopt;

    size_t idx = 0;
    try {
        idx = std::stoul(key.substr(num_start, num_end - num_start));
    } catch (...) { return std::nullopt; }

    // Case "INDEX1" -> Value is Chip Name
    if (num_end == key.size()) return {{idx, "INDEX"}};
    // Case "INDEX1_prop" -> Property for that block
    if (key[num_end] == '_') return {{idx, key.substr(num_end + 1)}};
    return std::nullopt;
}

L1Parser::RawDataMap L1Parser::parse_raw_config(const std::string& path) {
    std::ifstream file(path);
    RawDataMap raw_data;
    if (!file.is_open()) return raw_data;

    std::string line;
    while (std::getline(file, line)) {
        // Handle comments: strip text after '#' and trim whitespace
        std::string text = utils::trim(line.substr(0, line.find('#')));
        if (text.empty()) continue;

        size_t eq_pos = text.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = utils::trim(text.substr(0, eq_pos));
        std::string val = utils::trim(text.substr(eq_pos + 1));

        if (auto res = parse_index_key(key)) {
            raw_data[res->first][res->second] = val;
        }
    }
    return raw_data;
}

void L1Parser::process_block(size_t raw_idx, 
                              const std::unordered_map<std::string, std::string>& props,
                              std::unordered_map<std::string, size_t>& chipset_counter) 
{
    // Block must have an INDEX property (which contains the Chipset Name)
    if (props.find("INDEX") == props.end()) return;
    std::string chip_name = props.at("INDEX");

    // Update global counter for this specific chipset
    size_t main_idx = ++chipset_counter[chip_name];

    // Parse main_ifname (filter out empty entries)
    // Example: "ra0;rax0" -> ["ra0", "rax0"]
    std::string main_if_str = (props.count("main_ifname")) ? props.at("main_ifname") : "";
    std::vector<std::string> main_ifnames = utils::split(main_if_str, ';', false);

    if (main_ifnames.empty()) return;

    // Save RawBlock for idx2if (index to interface mapping) logic
    raw_blocks_.push_back({raw_idx, main_ifnames});

    // Iterate through each Band/Radio (Sub Index) found in this block
    for (size_t i = 0; i < main_ifnames.size(); ++i) {
        create_and_map_entry(chip_name, main_idx, i + 1, raw_idx, i, main_ifnames[i], props);
    }
}

void L1Parser::create_and_map_entry(const std::string& chip_name,
                                     size_t main_idx,
                                     size_t sub_idx,
                                     size_t raw_idx,
                                     size_t band_index,
                                     const std::string& current_main_if,
                                     const std::unordered_map<std::string, std::string>& props)
{
    // Lambda: Get a property from props, split by ';', and return the part corresponding to the current band.
    // Must keep empty tokens to maintain alignment.
    auto get_split_prop = [&](const std::string& k) -> std::string {
        if (props.find(k) != props.end()) {
            auto parts = utils::split(props.at(k), ';', true);
            if (band_index < parts.size()) return parts[band_index];
        }
        return "";
    };

    // Lambda: Logic to resolve default interface names if not specified in config
    size_t default_id = raw_idx + 1;
    auto resolve = [&](const std::string& k, const std::string& prefix, bool is_ext) -> std::string {
        std::string val = get_split_prop(k);
        if (!val.empty()) return val;
        // Fallback logic
        if (is_ext) return current_main_if + "_"; // e.g., ra0_1
        return prefix + std::to_string(default_id) + "_"; // e.g., apcli1_0
    };

    std::string ext_if = resolve("ext_ifname", "", true);
    std::string apcli_if = resolve("apcli_ifname", "apcli", false);
    std::string wds_if = resolve("wds_ifname", "wds", false);
    std::string mesh_if = resolve("mesh_ifname", "mesh", false);

    // Build Entry object
    L1Entry entry;
    entry.index_name = chip_name;
    entry.main_idx = main_idx;
    entry.sub_idx = sub_idx;

    // Fill properties
    for (const auto& kv : props) {
        const std::string& k = kv.first;
        // Copy direct properties (global to the block)
        if (k == "INDEX" || k.rfind("EEPROM", 0) == 0 || k == "mainidx") {
            entry.props[k] = kv.second;
        } else {
            // Split properties that are specific to a band
            entry.props[k] = get_split_prop(k);
        }
    }

    // Override calculated/derived properties
    entry.props["main_ifname"] = current_main_if;
    entry.props["ext_ifname"] = ext_if;
    entry.props["apcli_ifname"] = apcli_if;
    entry.props["wds_ifname"] = wds_if;
    entry.props["mesh_ifname"] = mesh_if;
    entry.props["subidx"] = std::to_string(sub_idx);
    entry.props["mainidx"] = std::to_string(main_idx);

    // Store in Profile map
    // Key format: "ChipName_MainIndex_SubIndex" (e.g., MT7981_1_1)
    std::string dev_key = chip_name + "_" + std::to_string(main_idx) + "_" + std::to_string(sub_idx);
    dev_map_[dev_key] = entry;
    ordered_dev_keys_.push_back(dev_key);

    // Create Reverse Map (Interface Name -> Entry)
    auto map_if = [&](const std::string& name) {
        if (!name.empty()) if_map_[name] = entry;
    };

    map_if(current_main_if);
    // Map virtual interfaces based on max counts
    for (size_t j = 1; j < MAX_NUM_EXTIF; ++j) map_if(ext_if + std::to_string(j));
    for (size_t j = 0; j < MAX_NUM_APCLI; ++j) map_if(apcli_if + std::to_string(j));
    for (size_t j = 0; j < MAX_NUM_WDS; ++j) map_if(wds_if + std::to_string(j));
    for (size_t j = 0; j < MAX_NUM_MESH; ++j) map_if(mesh_if + std::to_string(j));
}

std::optional<std::string> L1Parser::get_prop(const std::string& dev, const std::string& key) const {
    auto it = dev_map_.find(dev);
    if (it != dev_map_.end()) {
        auto pit = it->second.props.find(key);
        if (pit != it->second.props.end()) return pit->second;
    }
    return std::nullopt;
}

std::vector<std::string> L1Parser::list_devs() const {
    return ordered_dev_keys_;
}

std::optional<std::string> L1Parser::if2zone(const std::string& ifname) const {
    auto it = if_map_.find(ifname);
    if (it != if_map_.end()) {
        auto pit = it->second.props.find("nvram_zone");
        if (pit != it->second.props.end()) return pit->second;
    }
    return std::nullopt;
}

std::optional<std::string> L1Parser::if2dat(const std::string& ifname) const {
    auto it = if_map_.find(ifname);
    if (it != if_map_.end()) {
        auto pit = it->second.props.find("profile_path");
        if (pit != it->second.props.end()) return pit->second;
    }
    return std::nullopt;
}

std::optional<std::string> L1Parser::if2dbdcidx(const std::string& ifname) const {
    auto it = if_map_.find(ifname);
    if (it != if_map_.end()) {
        return std::to_string(it->second.sub_idx);
    }
    return std::nullopt;
}

std::vector<std::string> L1Parser::zone2if(const std::string& zone) const {
    std::vector<std::string> ifaces;
    for (const auto& pair : dev_map_) {
        const auto& props = pair.second.props;
        auto zit = props.find("nvram_zone");
        if (zit != props.end() && zit->second == zone) {
            auto add_if = [&](const char* k) {
                auto it = props.find(k);
                if (it != props.end() && !it->second.empty()) {
                    ifaces.push_back(it->second);
                }
            };
            add_if("main_ifname");
            add_if("ext_ifname");
            add_if("apcli_ifname");
            add_if("wds_ifname");
            add_if("mesh_ifname");

            return ifaces;
        }
    }
    return ifaces;
}

std::optional<std::string> L1Parser::idx2if(int idx) const {
    if (idx <= 0) return std::nullopt;
    size_t target = static_cast<size_t>(idx);
    size_t cumulative = 0;

    // Find which block matches the sequential index
    for (const auto& block : raw_blocks_) {
        size_t count = block.main_ifnames.size();
        if (target > cumulative && target <= cumulative + count) {
            size_t offset = target - cumulative - 1;
            if (offset < block.main_ifnames.size()) {
                return block.main_ifnames[offset];
            }
        }
        cumulative += count;
    }
    return std::nullopt;
}