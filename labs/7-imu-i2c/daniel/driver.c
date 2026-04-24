#include "rpi.h"
#include "mpu-6050.h"

uint8_t ADDR = 0b1101000;

void notmain(void) {
    mpu_init(ADDR);
    mpu_reset(ADDR);
    accel_t cfg = mpu_accel_config(ADDR, 8);
    mpu_self_test_accel(&cfg);
    gyro_t gyro_cfg = mpu_gyro_config(ADDR, 500);
    mpu_self_test_gyro(&gyro_cfg);

    return;
}
