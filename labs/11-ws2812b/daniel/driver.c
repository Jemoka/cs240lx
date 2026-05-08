#include "performance.h"
#include "timed.h"

void notmain(void) {
    lightbar_init();
    while (1) {
        lightbar_full(RED);
        delay_ms(100);
        lightbar_full(GREEN);
        delay_ms(100);
        lightbar_full(BLUE);
        delay_ms(100);
        lightbar_fronthalf(BLUE);
        delay_ms(500);
        lightbar_backhalf(BLUE);
        delay_ms(500);
        lightbar_alternate(BLUE);
        delay_ms(500);
        lightbar_alternate2(BLUE);
        delay_ms(500);

    }

}

