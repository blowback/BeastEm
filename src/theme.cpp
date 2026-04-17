#include "theme.hpp"

#include <cstdio>
#include <cctype>

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

static SDL_Color loadOrDefault(Config &config, const std::string &key, SDL_Color def, bool &dirty) {
    std::string formatted = formatColor(def);
    std::string current = config.getString(key, "");
    SDL_Color parsed;
    if (!current.empty() && parseHexColor(current, parsed)) {
        return parsed;
    }
    config.setString(key, formatted);
    dirty = true;
    return def;
}

Theme& Theme::instance() {
    static Theme t;
    return t;
}

void Theme::load(Config &config) {
    Theme &t = instance();
    bool dirty = false;

    t.text                = loadOrDefault(config, "color.text",                {0x00, 0x30, 0x30, 0xFF}, dirty);
    t.menu                = loadOrDefault(config, "color.menu",                {0x30, 0x30, 0xA0, 0xFF}, dirty);
    t.bright              = loadOrDefault(config, "color.bright",              {0xD0, 0xFF, 0xD0, 0xFF}, dirty);
    t.dim                 = loadOrDefault(config, "color.dim",                 {0x80, 0x80, 0x80, 0xFF}, dirty);
    t.very_dim            = loadOrDefault(config, "color.very_dim",            {0x60, 0x60, 0x60, 0xFF}, dirty);
    t.disabled            = loadOrDefault(config, "color.disabled",            {0x90, 0x90, 0xB0, 0xFF}, dirty);
    t.disassembly         = loadOrDefault(config, "color.disassembly",         {0x60, 0x00, 0x60, 0xFF}, dirty);
    t.current_pc          = loadOrDefault(config, "color.current_pc",          {0xA0, 0x30, 0x30, 0xFF}, dirty);
    t.yellow              = loadOrDefault(config, "color.yellow",              {0xFF, 0xFF, 0x80, 0xFF}, dirty);
    t.key_ink             = loadOrDefault(config, "color.key_ink",             {0xE0, 0xE0, 0xE0, 0xFF}, dirty);

    t.key_cap             = loadOrDefault(config, "color.key_cap",             {0x00, 0x00, 0x00, 0xFF}, dirty);
    t.keyboard_bg         = loadOrDefault(config, "color.keyboard_bg",         {0xE0, 0xE0, 0xF0, 0xFF}, dirty);
    t.key_pressed         = loadOrDefault(config, "color.key_pressed",         {0xA0, 0xA0, 0xA0, 0xFF}, dirty);

    t.breakpoint_enabled  = loadOrDefault(config, "color.breakpoint_enabled",  {0xDC, 0x32, 0x32, 0xFF}, dirty);
    t.breakpoint_disabled = loadOrDefault(config, "color.breakpoint_disabled", {0xDC, 0x32, 0x32, 0x64}, dirty);
    t.trace_enabled       = loadOrDefault(config, "color.trace_enabled",       {0xA0, 0x32, 0xDC, 0xFF}, dirty);
    t.trace_disabled      = loadOrDefault(config, "color.trace_disabled",      {0xA0, 0x32, 0xDC, 0x64}, dirty);
    t.watchpoint          = loadOrDefault(config, "color.watchpoint",          {0x00, 0xA0, 0xA0, 0xFF}, dirty);
    t.indicator_text      = loadOrDefault(config, "color.indicator_text",      {0xFF, 0xFF, 0xFF, 0xFF}, dirty);

    t.dialog_bg           = loadOrDefault(config, "color.dialog_bg",           {0xF0, 0xF0, 0xE0, 0xE8}, dirty);
    t.pagemap_bg          = loadOrDefault(config, "color.pagemap_bg",          {0xF0, 0xF0, 0xE0, 0xFF}, dirty);
    t.screen_bg           = loadOrDefault(config, "color.screen_bg",           {0x00, 0x00, 0x00, 0xFF}, dirty);

    t.edit_bg             = loadOrDefault(config, "color.edit_bg",             {0xFF, 0xFF, 0xFF, 0xFF}, dirty);
    t.edit_ink            = loadOrDefault(config, "color.edit_ink",            {0x00, 0x00, 0x00, 0xFF}, dirty);
    t.edit_highlight      = loadOrDefault(config, "color.edit_highlight",      {0xF0, 0x40, 0x40, 0xFF}, dirty);
    t.prompt_bg           = loadOrDefault(config, "color.prompt_bg",           {0xF0, 0xF0, 0xFF, 0xFF}, dirty);

    t.digit_bg            = loadOrDefault(config, "color.digit_bg",            {0x00, 0x00, 0x40, 0xFF}, dirty);
    t.digit_on            = loadOrDefault(config, "color.digit_on",            {0xF0, 0xF0, 0xFF, 0xFF}, dirty);

    t.page_addr           = loadOrDefault(config, "color.page_addr",           {0x60, 0x60, 0x60, 0xFF}, dirty);
    t.page_dark_text      = loadOrDefault(config, "color.page_dark_text",      {0x20, 0x20, 0x20, 0xFF}, dirty);
    t.page_light_text     = loadOrDefault(config, "color.page_light_text",     {0xFF, 0xFF, 0xFF, 0xFF}, dirty);
    t.page_border         = loadOrDefault(config, "color.page_border",         {0x40, 0x40, 0x40, 0xFF}, dirty);
    t.page_cpm            = loadOrDefault(config, "color.page_cpm",            {0x49, 0x93, 0xF3, 0xFF}, dirty);
    t.page_ramdisk        = loadOrDefault(config, "color.page_ramdisk",        {0x4D, 0xB2, 0xBB, 0xFF}, dirty);
    t.page_free           = loadOrDefault(config, "color.page_free",           {0xBC, 0xEC, 0xF1, 0xFF}, dirty);
    t.page_restore        = loadOrDefault(config, "color.page_restore",        {0xF6, 0x6B, 0x71, 0xFF}, dirty);
    t.page_romdisk        = loadOrDefault(config, "color.page_romdisk",        {0xB1, 0x4D, 0xB4, 0xFF}, dirty);
    t.page_boot           = loadOrDefault(config, "color.page_boot",           {0xF9, 0xD0, 0xFB, 0xFF}, dirty);
    t.page_vconsole       = loadOrDefault(config, "color.page_vconsole",       {0x2C, 0x22, 0xF2, 0xFF}, dirty);
    t.page_videobeast     = loadOrDefault(config, "color.page_videobeast",     {0x80, 0x80, 0x80, 0xFF}, dirty);

    if (dirty) {
        config.save();
    }
}
