#pragma once

#include "SDL.h"
#include "config.hpp"

struct Theme {
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
