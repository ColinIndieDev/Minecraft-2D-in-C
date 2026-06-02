#include "textures.h"
#include "items.h"

#ifndef __EMSCRIPTEN__
#include <cpl/cpl.h>
#else
#include "../external/cpl.h"
#endif

EXTERN_ITEMS_H_VARIABLES

texture item_textures[ITEM_TYPES];
texture hotbar;
texture hotbar_arrow;
texture inventory;
font f;

void textures_load_resources() {
    create_font(&f, "assets/fonts/default.ttf", "default", FILTER_LINEAR);

    for (u32 i = ITEM_NONE + 1; i < ITEM_TYPES; i++) {
        load_texture(&item_textures[i],
                 item_data[i].tex_path, FILTER_NEAREST);
    }

    load_texture(&hotbar, "assets/images/gui/hotbar.png", FILTER_NEAREST);
    load_texture(&hotbar_arrow, "assets/images/gui/hotbar_arrow.png",
                 FILTER_NEAREST);
    load_texture(&inventory, "assets/images/gui/inventory.png",
                 FILTER_NEAREST);
}
