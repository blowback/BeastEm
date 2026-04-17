#include "config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

static std::string trim(const std::string &s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    auto begin = std::find_if(s.begin(), s.end(), notSpace);
    auto end = std::find_if(s.rbegin(), s.rend(), notSpace).base();
    return (begin < end) ? std::string(begin, end) : std::string();
}

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

Config::Config(const std::string &path) : path(path) {
    std::ifstream in(path);
    if (!in) return;

    std::string line;
    while (std::getline(in, line)) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';') continue;

        auto eq = t.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(t.substr(0, eq));
        std::string value = trim(t.substr(eq + 1));
        if (!key.empty()) values[key] = value;
    }
}

bool Config::getBool(const std::string &key, bool defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;
    std::string v = toLower(it->second);
    if (v == "true" || v == "yes" || v == "on" || v == "1") return true;
    if (v == "false" || v == "no" || v == "off" || v == "0") return false;
    return defaultValue;
}

std::string Config::getString(const std::string &key, const std::string &defaultValue) const {
    auto it = values.find(key);
    return (it == values.end()) ? defaultValue : it->second;
}

void Config::setBool(const std::string &key, bool value) {
    values[key] = value ? "true" : "false";
}

void Config::setString(const std::string &key, const std::string &value) {
    values[key] = value;
}

bool Config::save() const {
    std::ofstream out(path);
    if (!out) return false;
    for (const auto &kv : values) {
        out << kv.first << " = " << kv.second << "\n";
    }
    return static_cast<bool>(out);
}
