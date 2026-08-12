#include "textures.h"

texture_t item_textures[ITEM_TYPE_T_SIZE];
texture_t ui_textures[TEXTURE_TYPE_T_SIZE];
font_t f;

void textures_load_resources() {
    font_load(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    for (uint32_t t = ITEM_NONE + 1; t < ITEM_TYPE_T_SIZE; t++) {
        texture_load(&item_textures[t], items_get_item_data(t)->tex_path, FILTER_NEAREST);
    }

    texture_load(&ui_textures[TEXTURE_HOTBAR], "assets/images/gui/hotbar.png", FILTER_NEAREST);
    texture_load(&ui_textures[TEXTURE_HOTBAR_ARROW], "assets/images/gui/hotbar_arrow.png", FILTER_NEAREST);
    texture_load(&ui_textures[TEXTURE_INVENTORY], "assets/images/gui/inventory.png", FILTER_NEAREST);
}

texture_t *textures_get_item_texture(item_type_t type) {
    return &item_textures[type];
}

texture_t *textures_get_ui_texture(texture_type_t type) {
    return &ui_textures[type];
}

font_t *textures_get_font() {
    return &f;
}
