#ifndef __performance_h__
#define __performance_h__

#include "rpi.h"
#include "ws2812b.h"
#include "gpio.h"

#define SIZE 70
#define PIN 21
#define RED rgb(255, 0, 0)
#define GREEN rgb(0, 255, 0)
#define BLUE rgb(0, 0, 255)

#define WIDTH 10
#define HEIGHT 7
#define LS 2
#define RS 1

static inline void lightbar_init() {
    cycle_cnt_init();
    gpio_set_output(PIN);
}

static inline void lightbar_full(uint32_t color) {
    uint32_t buf[SIZE];
    for (unsigned i = 0; i < SIZE; i++) {
        buf[i] = color;
    }
    ws_send_buffer(PIN, buf, SIZE);
}

static inline void lightbar_fronthalf(uint32_t color) {
    uint32_t buf[SIZE];
    for (unsigned i = 0; i < SIZE/2; i++) {
        buf[i] = color;
    }
    for (unsigned i = SIZE/2; i < SIZE; i++) {
        buf[i] = 0;
    }
    ws_send_buffer(PIN, buf, SIZE);
}
static inline void lightbar_backhalf(uint32_t color) {
    uint32_t buf[SIZE];
    for (unsigned i = 0; i < SIZE/2; i++) {
        buf[i] = 0;
    }
    for (unsigned i = SIZE/2; i < SIZE; i++) {
        buf[i] = color;
    }
    ws_send_buffer(PIN, buf, SIZE);
}
static inline void lightbar_alternate(uint32_t color) {
    uint32_t buf[SIZE];
    for (unsigned i = 0; i < SIZE; i++) {
        if (i % 2 == 0) {
            buf[i] = color;
        } else {
            buf[i] = 0;
        }
    }
    ws_send_buffer(PIN, buf, SIZE);
}
static inline void lightbar_alternate2(uint32_t color) {
    uint32_t buf[SIZE];
    for (unsigned i = 0; i < SIZE; i++) {
        if (i % 2 == 1) {
            buf[i] = color;
        } else {
            buf[i] = 0;
        }
    }
    ws_send_buffer(PIN, buf, SIZE);
}


#endif
