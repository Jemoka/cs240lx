#include "prelude.h"

void notmain(void) {
    eraser_init();
    eraser_set_thread_id(1);

    // mmmmmm you can indeed seta value wow
    int *y = gcmalloc(4);
    *y = 1;
    assert(*y == 1);

    trace("SUCCESS\n");
}

