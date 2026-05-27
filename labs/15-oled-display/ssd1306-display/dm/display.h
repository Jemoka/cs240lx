#ifndef __display__h__
#define __display__h__

#include "rpi.h"
#include "i2c.h"

void draw_pixel(int16_t x, int16_t y, uint8_t color);
void clear(void);

void fill(uint8_t pattern);

void display_update(uint16_t column_s, uint16_t column_e,
                    uint16_t page_s, uint16_t page_e);

void display_draw(void);

   

void init(void);

void draw_full_frame(uint8_t* frame);

#endif
