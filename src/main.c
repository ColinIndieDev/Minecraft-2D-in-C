#include "core.h"

int main(void) {
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!window_should_close()) {
        main_loop();
    }
#endif

    terminate();    
}
