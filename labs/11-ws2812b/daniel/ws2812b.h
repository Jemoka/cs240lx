#ifndef __ws281b_h__
#define __ws281b_h__

#include "timed.h"
#pragma GCC optimize ("align-functions=16")

static inline void ws_zero(unsigned pin) {
    // high for 0.35us, low for 0.9us
    write_1(pin, 350);
    write_0(pin, 900);
}
static inline void ws_one(unsigned pin) {
    // high for 0.9us, low for 0.35us
    write_1(pin, 900);
    write_0(pin, 350);
}
static inline void ws_reset(unsigned pin) {
    // low for 50us
    write_0(pin, 100000);
}

// for sending a single RGB value, where the 24 bits are RRRRRRRR GGGGGGGG BBBBBBBB
static inline void ws_send(unsigned pin, uint32_t data) {
    for (int i = 0; i < 24; i++) {
        if ((data >> i) & 0b1) {
            ws_one(pin);
        } else {
            ws_zero(pin);
        }
    }
}
// encode RGB 
static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    // rrrrrrrr gggggggg bbbbbbbb
    return ((uint32_t)b << 16) | ((uint32_t)r << 8) | (uint32_t)g;
}

// send a single buffer of RGB values and the reset
static inline void ws_send_buffer(unsigned pin, uint32_t * data, unsigned n) {
    for (unsigned i = 0; i < n; i++) {
        ws_send(pin, data[i]);
    }
    ws_reset(pin);
}

#endif
