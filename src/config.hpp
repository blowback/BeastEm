#pragma once

#include <map>
#include <string>

class Config {
public:
    explicit Config(const std::string &path);

    bool getBool(const std::string &key, bool defaultValue) const;
    std::string getString(const std::string &key, const std::string &defaultValue) const;

    void setBool(const std::string &key, bool value);
    void setString(const std::string &key, const std::string &value);

    bool save() const;

private:
    std::string path;
    std::map<std::string, std::string> values;
};
