#include "rpi.h"
#include "i2c.h"
#include "display.h"

extern const uint8_t frames_data[];
extern const uint8_t frames_data_end[];

#define FRAME_BYTES  1024u
#define FRAME_COUNT  ((uint32_t)(frames_data_end - frames_data) / FRAME_BYTES)

static inline const uint8_t *frame_ptr(uint32_t i) {
    return frames_data + i * FRAME_BYTES;
}

// print dots each frame, 120 x 64, 8 pixels per byte, so 120/8 = 15 bytes per row, 64 rows, so 15*64 = 960 bytes per frame
void debug_print_frame(const uint8_t *frame) {
    for (int row = 0; row < 64; row++) {
        for (int col_byte = 0; col_byte < 16; col_byte++) {        // 16, not 15
            uint8_t byte = frame[row * 16 + col_byte];             // *16, not *15
            for (int bit = 0; bit < 8; bit++) {
                if (byte & (0x80 >> bit)) {                        // MSB-first
                    printk("*");
                } else {
                    printk(" ");
                }
            }
        }
        printk("\n");
    }
}


void notmain(void) {
    delay_ms(100);
    i2c_init_clk_div(1500);
    delay_ms(100);

    init();

    clear();
    display_draw();

    /* debug_print_frame(frame_ptr(40)); */



    // print the first frame

    // debug print
    while (1) {
        for (int i = 5; i < FRAME_COUNT; i++) {
            draw_full_frame((uint8_t *) frame_ptr(i));
            display_draw();
            printk("Displayed frame %d, first byte: %x\n", i, frame_ptr(i)[0]);

            /* const uint8_t *frame = frame_ptr(i); */
            /* printk("Frame %d: %02x %02x %02x ...\n", i, frame[0], frame[1], frame[2]); */
        }
    }

    /* while (1) { */
    /*     display_draw(); */
    /*     clear(); */
    /*     display_draw(); */
    /*     for (int i = 0; i < FRAME_COUNT; i++) { */
    /*         draw_full_frame((uint8_t *) frame_ptr(i)); */
    /*         display_draw(); */
    /*     } */
    /* } */
    
}

