#pragma once

#include <map>
#include <string>
#include <vector>

class Config {
public:
    explicit Config(const std::string &path);

    // Top-level (no section) lookups
    bool getBool(const std::string &key, bool defaultValue) const;
    std::string getString(const std::string &key, const std::string &defaultValue) const;
    void setBool(const std::string &key, bool value);
    void setString(const std::string &key, const std::string &value);

    // Section-aware lookups. Section name excludes the surrounding brackets.
    std::string getString(const std::string &section, const std::string &key, const std::string &defaultValue) const;
    void setString(const std::string &section, const std::string &key, const std::string &value);

    std::vector<std::string> keys(const std::string &section) const;
    void erase(const std::string &section, const std::string &key);

    bool save() const;

private:
    std::string path;
    // sections[""]             = top-level keys
    // sections["theme.default"] = keys within the [theme.default] section
    std::map<std::string, std::map<std::string, std::string>> sections;
};
