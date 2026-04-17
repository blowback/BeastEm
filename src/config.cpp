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

    std::string currentSection;
    std::string line;
    while (std::getline(in, line)) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';') continue;

        if (t.front() == '[' && t.back() == ']') {
            currentSection = trim(t.substr(1, t.size() - 2));
            continue;
        }

        auto eq = t.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(t.substr(0, eq));
        std::string value = trim(t.substr(eq + 1));
        if (!key.empty()) sections[currentSection][key] = value;
    }
}

bool Config::getBool(const std::string &key, bool defaultValue) const {
    auto sit = sections.find("");
    if (sit == sections.end()) return defaultValue;
    auto it = sit->second.find(key);
    if (it == sit->second.end()) return defaultValue;
    std::string v = toLower(it->second);
    if (v == "true" || v == "yes" || v == "on" || v == "1") return true;
    if (v == "false" || v == "no" || v == "off" || v == "0") return false;
    return defaultValue;
}

std::string Config::getString(const std::string &key, const std::string &defaultValue) const {
    return getString("", key, defaultValue);
}

std::string Config::getString(const std::string &section, const std::string &key, const std::string &defaultValue) const {
    auto sit = sections.find(section);
    if (sit == sections.end()) return defaultValue;
    auto it = sit->second.find(key);
    return (it == sit->second.end()) ? defaultValue : it->second;
}

void Config::setBool(const std::string &key, bool value) {
    sections[""][key] = value ? "true" : "false";
}

void Config::setString(const std::string &key, const std::string &value) {
    sections[""][key] = value;
}

void Config::setString(const std::string &section, const std::string &key, const std::string &value) {
    sections[section][key] = value;
}

std::vector<std::string> Config::keys(const std::string &section) const {
    std::vector<std::string> result;
    auto sit = sections.find(section);
    if (sit == sections.end()) return result;
    result.reserve(sit->second.size());
    for (const auto &kv : sit->second) result.push_back(kv.first);
    return result;
}

void Config::erase(const std::string &section, const std::string &key) {
    auto sit = sections.find(section);
    if (sit == sections.end()) return;
    sit->second.erase(key);
}

bool Config::save() const {
    std::ofstream out(path);
    if (!out) return false;

    // Emit top-level (section "") first so that top-level keys come before any [section] header.
    auto topIt = sections.find("");
    if (topIt != sections.end()) {
        for (const auto &kv : topIt->second) {
            out << kv.first << " = " << kv.second << "\n";
        }
    }

    for (const auto &sectionEntry : sections) {
        if (sectionEntry.first.empty()) continue;
        out << "\n[" << sectionEntry.first << "]\n";
        for (const auto &kv : sectionEntry.second) {
            out << kv.first << " = " << kv.second << "\n";
        }
    }
    return static_cast<bool>(out);
}
