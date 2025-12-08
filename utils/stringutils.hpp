#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace utils {

// Remove leading and trailing whitespace characters (\t, \n, \r, space)
inline std::string trim(const std::string& str) {
    const auto strBegin = str.find_first_not_of(" \t\n\r");
    if (strBegin == std::string::npos) return "";
    const auto strEnd = str.find_last_not_of(" \t\n\r");
    return str.substr(strBegin, strEnd - strBegin + 1);
}

/**
 * @brief split function designed to filter profile values.
 * @param s Input string.
 * @param delimiter The character to split by.
 * @param keep_empty 
 *      true: Preserves empty elements. 
 *            Used for property alignment (e.g., "val1;;val3" -> ["val1", "", "val3"]).
 *      false: Filters out empty elements.
 *            Used for main_ifname parsing where gaps don't matter.
 */
inline std::vector<std::string> split(const std::string& s, char delimiter, bool keep_empty) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    // Use getline to process the stream
    while (std::getline(tokenStream, token, delimiter)) {
        std::string trimmed = trim(token);
        if (keep_empty || !trimmed.empty()) {
            tokens.push_back(trimmed);
        }
    }

    // Special case: If keep_empty is true and the string ends with the delimiter (e.g., "a;b;")
    // std::getline ignores the final empty slot, so we manually add it.
    if (keep_empty && !s.empty() && s.back() == delimiter) {
        tokens.push_back("");
    }

    return tokens;
}

}
