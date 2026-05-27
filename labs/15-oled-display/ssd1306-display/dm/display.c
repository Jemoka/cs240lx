#include "rpi.h"
#include "i2c.h"
#include "display.h"

#define SSD1306_ADDR 0x3C
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64
#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

static uint8_t framebuffer[SSD1306_BUFFER_SIZE];  // 1024 bytes

void draw_pixel(int16_t x, int16_t y, uint8_t color) {
    if ((x < 0) || (x >= SSD1306_WIDTH) || 
        (y < 0) || (y >= SSD1306_HEIGHT)) {
        demand(0, "Pixel out of bounds: (%d, %d)", x, y);
    }
    
    uint16_t byte_index = x + ((y >> 3) * SSD1306_WIDTH);
    
    uint8_t bit_mask = 1 << (y & 7);
    
    if (color) {
        framebuffer[byte_index] |= bit_mask;   // Set (OR)
    } else {
        framebuffer[byte_index] &= ~bit_mask;  // Clear (AND NOT)
    }
}
void clear(void) {
    memset(framebuffer, 0x00, SSD1306_BUFFER_SIZE);
}

void fill(uint8_t pattern) {
    memset(framebuffer, pattern, SSD1306_BUFFER_SIZE);
}

void display_update(uint16_t column_s, uint16_t column_e,
                    uint16_t page_s, uint16_t page_e) {
    uint8_t setup[] = {
        0x00,
        0x20, 0x00,
        0x21, column_s, column_e,
        0x22, page_s, page_e
    };
    i2c_write(SSD1306_ADDR, setup, sizeof(setup));
    
    for (uint16_t page = page_s; page <= page_e; page++) {
        for (uint16_t col = column_s; col <= column_e; col += 16) {
            uint8_t chunk[17];
            chunk[0] = 0x40;
            uint16_t byte_index = col + (page * SSD1306_WIDTH);
            memcpy(&chunk[1], &framebuffer[byte_index], 16);
            i2c_write(SSD1306_ADDR, chunk, 17);
        }
    }
}
void display_draw(void) {
    display_update(0, 127, 0, 7);
}




    

void init(void) {
    //// init screen //// 
    uint8_t init[] = {
        0x00,        // Control byte: command stream
        0xAE,        // Display OFF
        0x8D, 0x14,  // Enable charge pump (CRITICAL!)
        0xAF,        // Display ON
        0xA4         // Ignore RAM, all pixels ON (test mode)
    };
    i2c_write(SSD1306_ADDR, init, sizeof(init));

    //// clear screen //// 
    uint8_t clear_cmd[] = {
        0x00, // control  
        0x20, // page addressing
        0x00, // horizonal addressing
        0x21, // column address range
        0x00, //  ? typically
        0x7F, //
        0x22, // page address range
        0x00, // ? typically
        0x07  // 
    };
    i2c_write(SSD1306_ADDR, clear_cmd, sizeof(clear_cmd));
    uint8_t clear_data[1025];
    clear_data[0] = 0x40;  // Data stream control byte
    memset(&clear_data[1], 0x00, 1024);  // All pixels OFF
    i2c_write(0x3C, clear_data, 1025);
}

// top to bottom, left to right
void draw_full_frame(uint8_t* frame) {
    for (int row = 0; row < 64; row++) {
        for (int col_byte = 0; col_byte < 16; col_byte++) {        // 16, not 15
            uint8_t byte = frame[row * 16 + col_byte];             // *16, not *15
            for (int bit = 0; bit < 8; bit++) {
                uint8_t color = (byte & (0x80 >> bit)) ? 1 : 0;   // MSB-first
                int x = (col_byte * 8) + bit;
                int y = row;
                draw_pixel(x, y, color);
            }
        }
    }
}
