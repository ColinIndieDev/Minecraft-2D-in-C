#include "textures.h"
#include "items.h"

#include <cpl/cpl.h>

EXTERN_ITEMS_H_VARIABLES

texture item_textures[ITEM_TYPE_T_SIZE];
texture hotbar;
texture hotbar_arrow;
texture inventory;
font f;

void textures_load_resources() {
    font_load(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    for (uint32_t t = ITEM_NONE + 1; t < ITEM_TYPE_T_SIZE; t++) {
        texture_load(&item_textures[t],
                 item_data[t].tex_path, FILTER_NEAREST);
    }

    texture_load(&hotbar, "assets/images/gui/hotbar.png", FILTER_NEAREST);
    texture_load(&hotbar_arrow, "assets/images/gui/hotbar_arrow.png", FILTER_NEAREST);
    texture_load(&inventory, "assets/images/gui/inventory.png", FILTER_NEAREST);
}
