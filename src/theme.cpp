#include "theme.hpp"

#include "assets.hpp"

#include <cstdio>
#include <cctype>
#include <fstream>

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool parseHexColor(const std::string &s, SDL_Color &out) {
    if (s.empty() || s[0] != '#') return false;
    size_t len = s.size() - 1;
    if (len != 6 && len != 8) return false;
    int digits[8];
    for (size_t i = 0; i < len; i++) {
        int v = hexVal(s[i + 1]);
        if (v < 0) return false;
        digits[i] = v;
    }
    out.r = (Uint8)((digits[0] << 4) | digits[1]);
    out.g = (Uint8)((digits[2] << 4) | digits[3]);
    out.b = (Uint8)((digits[4] << 4) | digits[5]);
    out.a = (len == 8) ? (Uint8)((digits[6] << 4) | digits[7]) : 0xFF;
    return true;
}

static std::string formatColor(SDL_Color c) {
    char buf[12];
    if (c.a == 0xFF) {
        snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    } else {
        snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
    }
    return std::string(buf);
}

static bool fileExists(const std::string &path) {
    std::ifstream f(path);
    return f.good();
}

static bool looksLikePath(const std::string &s) {
    if (s.empty()) return false;
    if (s[0] == '/' || s[0] == '.' || s[0] == '~') return true;
    return s.find('/') != std::string::npos;
}

static bool sectionHasColors(const Config &config, const std::string &section) {
    for (const std::string &key : config.keys(section)) {
        if (key.rfind("color.", 0) == 0) return true;
    }
    return false;
}

// Resolution order for active_theme value:
//   1. path-like (contains / or starts with . / ~) → read from that file
//   2. [theme.<name>] section in user config with any color.* entries → use it
//   3. assets/themes/<name>.theme → read from that file
//   4. otherwise → create [theme.<name>] section in user config and seed defaults
//
// File-based themes are read-only: missing keys fall back to hardcoded defaults
// without being written anywhere. Config-based themes write missing keys back
// so the user sees every knob.
struct ThemeSource {
    enum class Mode { Section, File };
    Mode mode;
    Config *userConfig = nullptr;   // always set (top-level override source + Section write-back)
    Config *fileConfig = nullptr;   // set in File mode
    std::string section;            // set in Section mode

    SDL_Color get(const std::string &key, SDL_Color def, bool &dirty) const {
        // Top-level override in user config wins over everything.
        std::string override = userConfig->getString("", key, "");
        SDL_Color parsed;
        if (!override.empty() && parseHexColor(override, parsed)) return parsed;

        if (mode == Mode::File) {
            std::string current = fileConfig->getString("", key, "");
            if (!current.empty() && parseHexColor(current, parsed)) return parsed;
            return def;
        }
        std::string current = userConfig->getString(section, key, "");
        if (!current.empty() && parseHexColor(current, parsed)) return parsed;
        userConfig->setString(section, key, formatColor(def));
        dirty = true;
        return def;
    }

    std::string getStr(const std::string &key, const std::string &def, bool &dirty) const {
        std::string override = userConfig->getString("", key, "");
        if (!override.empty()) return override;

        if (mode == Mode::File) {
            std::string current = fileConfig->getString("", key, "");
            return current.empty() ? def : current;
        }
        std::string current = userConfig->getString(section, key, "");
        if (!current.empty()) return current;
        userConfig->setString(section, key, def);
        dirty = true;
        return def;
    }
};

std::string fontPath(const std::string &name) {
    if (name.empty()) return "";
    if (name[0] == '/' || name[0] == '.' || name[0] == '~') return name;
    return assetPath("fonts/" + name);
}

Theme& Theme::instance() {
    static Theme t;
    return t;
}

void Theme::load(Config &config) {
    Theme &t = instance();
    bool dirty = false;

    std::string active = config.getString("active_theme", "");
    bool freshInstall = active.empty();
    if (freshInstall) {
        active = "default";
        config.setString("active_theme", active);
        dirty = true;
    }

    // Migrate legacy flat "color.*" keys into [theme.default] on first upgrade.
    if (freshInstall) {
        const std::string defaultSection = "theme.default";
        for (const std::string &key : config.keys("")) {
            if (key.rfind("color.", 0) == 0) {
                std::string val = config.getString("", key, "");
                if (!val.empty() && config.getString(defaultSection, key, "").empty()) {
                    config.setString(defaultSection, key, val);
                }
                config.erase("", key);
                dirty = true;
            }
        }
    }

    ThemeSource src;
    src.userConfig = &config;  // always set — top-level overrides come from here
    Config fileConfig(""); // populated below if needed

    auto tryLoadFile = [&](const std::string &path) -> bool {
        if (!fileExists(path)) return false;
        fileConfig = Config(path);
        src.mode = ThemeSource::Mode::File;
        src.fileConfig = &fileConfig;
        return true;
    };

    if (looksLikePath(active)) {
        if (!tryLoadFile(active)) {
            // Path given but file missing → fall back to default section
            src.mode = ThemeSource::Mode::Section;
            src.userConfig = &config;
            src.section = "theme.default";
        }
    } else {
        const std::string section = "theme." + active;
        if (sectionHasColors(config, section)) {
            src.mode = ThemeSource::Mode::Section;
            src.userConfig = &config;
            src.section = section;
        } else if (tryLoadFile(assetPath("themes/" + active + ".theme"))) {
            // Loaded from shipped asset
        } else {
            // New/unknown name → seed a new section with defaults
            src.mode = ThemeSource::Mode::Section;
            src.userConfig = &config;
            src.section = section;
        }
    }

    t.font_ui               = src.getStr("font.ui",               "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_ui_labels        = src.getStr("font.ui.labels",        "Roboto-Medium.ttf", dirty);
    t.font_ui_indicators    = src.getStr("font.ui.indicators",    "Roboto-Medium.ttf", dirty);
    t.font_ui_mem_address   = src.getStr("font.ui.mem.address",   "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_ui_mem_hex       = src.getStr("font.ui.mem.hex",       "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_ui_mem_chars     = src.getStr("font.ui.mem.chars",     "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_ui_keyhint_key   = src.getStr("font.ui.keyhint.key",   "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_ui_keyhint_hint  = src.getStr("font.ui.keyhint.hint",  "RobotoMono-VariableFont_wght.ttf", dirty);
    t.font_key_caps         = src.getStr("font.key.caps",         "Roboto-Medium.ttf", dirty);
    t.font_key_narrow       = src.getStr("font.key.narrow",       "Roboto-Medium.ttf", dirty);
    t.font_key_modifiers    = src.getStr("font.key.modifiers",    "Roboto-Medium.ttf", dirty);
    t.font_pagemap_label    = src.getStr("font.pagemap.label",    "Roboto-Medium.ttf", dirty);
    t.font_pagemap_address  = src.getStr("font.pagemap.address",  "Roboto-Medium.ttf", dirty);

    t.text                = src.get("color.text",                {0x00, 0x30, 0x30, 0xFF}, dirty);
    t.menu                = src.get("color.menu",                {0x30, 0x30, 0xA0, 0xFF}, dirty);
    t.bright              = src.get("color.bright",              {0xD0, 0xFF, 0xD0, 0xFF}, dirty);
    t.dim                 = src.get("color.dim",                 {0x80, 0x80, 0x80, 0xFF}, dirty);
    t.very_dim            = src.get("color.very_dim",            {0x60, 0x60, 0x60, 0xFF}, dirty);
    t.disabled            = src.get("color.disabled",            {0x90, 0x90, 0xB0, 0xFF}, dirty);
    t.disassembly         = src.get("color.disassembly",         {0x60, 0x00, 0x60, 0xFF}, dirty);
    t.current_pc          = src.get("color.current_pc",          {0xA0, 0x30, 0x30, 0xFF}, dirty);
    t.yellow              = src.get("color.yellow",              {0xFF, 0xFF, 0x80, 0xFF}, dirty);
    t.key_ink             = src.get("color.key_ink",             {0xE0, 0xE0, 0xE0, 0xFF}, dirty);

    t.key_cap             = src.get("color.key_cap",             {0x00, 0x00, 0x00, 0xFF}, dirty);
    t.keyboard_bg         = src.get("color.keyboard_bg",         {0xE0, 0xE0, 0xF0, 0xFF}, dirty);
    t.key_pressed         = src.get("color.key_pressed",         {0xA0, 0xA0, 0xA0, 0xFF}, dirty);

    t.breakpoint_enabled  = src.get("color.breakpoint_enabled",  {0xDC, 0x32, 0x32, 0xFF}, dirty);
    t.breakpoint_disabled = src.get("color.breakpoint_disabled", {0xDC, 0x32, 0x32, 0x64}, dirty);
    t.trace_enabled       = src.get("color.trace_enabled",       {0xA0, 0x32, 0xDC, 0xFF}, dirty);
    t.trace_disabled      = src.get("color.trace_disabled",      {0xA0, 0x32, 0xDC, 0x64}, dirty);
    t.watchpoint          = src.get("color.watchpoint",          {0x00, 0xA0, 0xA0, 0xFF}, dirty);
    t.indicator_text      = src.get("color.indicator_text",      {0xFF, 0xFF, 0xFF, 0xFF}, dirty);

    t.dialog_bg           = src.get("color.dialog_bg",           {0xF0, 0xF0, 0xE0, 0xE8}, dirty);
    t.pagemap_bg          = src.get("color.pagemap_bg",          {0xF0, 0xF0, 0xE0, 0xFF}, dirty);
    t.screen_bg           = src.get("color.screen_bg",           {0x00, 0x00, 0x00, 0xFF}, dirty);

    t.edit_bg             = src.get("color.edit_bg",             {0xFF, 0xFF, 0xFF, 0xFF}, dirty);
    t.edit_ink            = src.get("color.edit_ink",            {0x00, 0x00, 0x00, 0xFF}, dirty);
    t.edit_highlight      = src.get("color.edit_highlight",      {0xF0, 0x40, 0x40, 0xFF}, dirty);
    t.prompt_bg           = src.get("color.prompt_bg",           {0xF0, 0xF0, 0xFF, 0xFF}, dirty);

    t.digit_bg            = src.get("color.digit_bg",            {0x00, 0x00, 0x40, 0xFF}, dirty);
    t.digit_on            = src.get("color.digit_on",            {0xF0, 0xF0, 0xFF, 0xFF}, dirty);

    t.page_addr           = src.get("color.page_addr",           {0x60, 0x60, 0x60, 0xFF}, dirty);
    t.page_dark_text      = src.get("color.page_dark_text",      {0x20, 0x20, 0x20, 0xFF}, dirty);
    t.page_light_text     = src.get("color.page_light_text",     {0xFF, 0xFF, 0xFF, 0xFF}, dirty);
    t.page_border         = src.get("color.page_border",         {0x40, 0x40, 0x40, 0xFF}, dirty);
    t.page_cpm            = src.get("color.page_cpm",            {0x49, 0x93, 0xF3, 0xFF}, dirty);
    t.page_ramdisk        = src.get("color.page_ramdisk",        {0x4D, 0xB2, 0xBB, 0xFF}, dirty);
    t.page_free           = src.get("color.page_free",           {0xBC, 0xEC, 0xF1, 0xFF}, dirty);
    t.page_restore        = src.get("color.page_restore",        {0xF6, 0x6B, 0x71, 0xFF}, dirty);
    t.page_romdisk        = src.get("color.page_romdisk",        {0xB1, 0x4D, 0xB4, 0xFF}, dirty);
    t.page_boot           = src.get("color.page_boot",           {0xF9, 0xD0, 0xFB, 0xFF}, dirty);
    t.page_vconsole       = src.get("color.page_vconsole",       {0x2C, 0x22, 0xF2, 0xFF}, dirty);
    t.page_videobeast     = src.get("color.page_videobeast",     {0x80, 0x80, 0x80, 0xFF}, dirty);

    if (dirty) {
        config.save();
    }
}
