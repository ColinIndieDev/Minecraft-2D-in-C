#pragma once

#include "items.h"

#include <stdint.h>
#include <cpl/cpl.h>

typedef enum : uint8_t {
    TEXTURE_HOTBAR = 0,
    TEXTURE_INVENTORY,
    TEXTURE_HOTBAR_ARROW,
    TEXTURE_TYPE_T_SIZE
} texture_type_t;

void textures_load_resources();
texture_t *textures_get_item_texture(item_type_t type);
texture_t *textures_get_ui_texture(texture_type_t type);
font_t *textures_get_font();
