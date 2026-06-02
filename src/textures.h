#pragma once

#define EXTERN_TEXTURES_H_VARIABLES                                            \
    extern texture item_textures[ITEM_TYPES];                                  \
    extern texture hotbar;                                                     \
    extern texture hotbar_arrow;                                               \
    extern font f;                                                             \
    extern texture inventory;

void textures_load_resources();
