#include "rpi.h"
#include "mpu-6050.h"

uint8_t ADDR = 0b1101000;

void notmain(void) {
    mpu_init(ADDR);
    mpu_reset(ADDR);
    accel_t cfg = mpu_accel_config(ADDR, 8);
    while (1) {
        triple_t t = mpu_accel_read(&cfg);
        printk("accel: x = %d mg, y = %d mg, z = %d mg\n", t.x, t.y, t.z);
        delay_ms(1000);
    }

    return;
}
