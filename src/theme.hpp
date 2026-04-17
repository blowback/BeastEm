#pragma once

#include <string>

#include "SDL.h"
#include "config.hpp"

struct Theme {
    // Fonts (filenames relative to assets/fonts/, or absolute paths).
    // Semantic slots — each UI region can use a different font.
    std::string font_ui;                // the whole debug GUI (monospace)
    std::string font_ui_labels;         // register / column / port labels in the debug view
    std::string font_ui_indicators;     // breakpoint / watchpoint number badges
    std::string font_ui_mem_address;    // memory viewer: "0x####" address column
    std::string font_ui_mem_hex;        // memory viewer: hex byte dump
    std::string font_ui_mem_chars;      // memory viewer: ASCII character dump
    std::string font_key_caps;          // main label on virtual keyboard keys
    std::string font_key_narrow;        // narrow keys (Shift, Ctrl, …)
    std::string font_key_modifiers;     // secondary labels (Shift+X / Ctrl+X)
    std::string font_pagemap_label;     // pagemap titles + page numbers
    std::string font_pagemap_address;   // pagemap small address labels

    // Common UI
    SDL_Color text;
    SDL_Color menu;
    SDL_Color bright;
    SDL_Color dim;
    SDL_Color very_dim;
    SDL_Color disabled;
    SDL_Color disassembly;
    SDL_Color current_pc;
    SDL_Color yellow;
    SDL_Color key_ink;

    // Keyboard
    SDL_Color key_cap;
    SDL_Color keyboard_bg;
    SDL_Color key_pressed;

    // Indicators
    SDL_Color breakpoint_enabled;
    SDL_Color breakpoint_disabled;
    SDL_Color trace_enabled;
    SDL_Color trace_disabled;
    SDL_Color watchpoint;
    SDL_Color indicator_text;

    // Overlays / backgrounds
    SDL_Color dialog_bg;       // cream w/ ~0xE8 alpha
    SDL_Color pagemap_bg;      // cream solid
    SDL_Color screen_bg;       // black behind scene

    // Edit / prompt
    SDL_Color edit_bg;
    SDL_Color edit_ink;
    SDL_Color edit_highlight;
    SDL_Color prompt_bg;

    // Digit display
    SDL_Color digit_bg;
    SDL_Color digit_on;

    // Pagemap
    SDL_Color page_addr;
    SDL_Color page_dark_text;
    SDL_Color page_light_text;
    SDL_Color page_border;
    SDL_Color page_cpm;
    SDL_Color page_ramdisk;
    SDL_Color page_free;
    SDL_Color page_restore;
    SDL_Color page_romdisk;
    SDL_Color page_boot;
    SDL_Color page_vconsole;
    SDL_Color page_videobeast;

    static Theme& instance();
    static void load(Config &config);
};

// Resolves a font name to a full asset path. If `name` is an absolute path
// or starts with ./ / ~/ it is returned as-is; otherwise prefixed with
// assets/fonts/.
std::string fontPath(const std::string &name);
