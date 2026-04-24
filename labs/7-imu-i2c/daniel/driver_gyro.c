#include "rpi.h"
#include "mpu-6050.h"

uint8_t ADDR = 0b1101000;

void notmain(void) {
    mpu_init(ADDR);
    mpu_reset(ADDR);

    gyro_t gyro = mpu_gyro_config(ADDR, 250);
    while (1) {
        triple_t g = mpu_gyro_read(&gyro);
        printk("gyro: x = %d, y = %d, z = %d\n", g.x, g.y, g.z);
        delay_ms(100);
    }


    return;
}
