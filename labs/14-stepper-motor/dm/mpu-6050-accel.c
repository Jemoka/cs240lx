#include "mpu-6050.h"
#include "mpu-regs.h"
#include "bit-support.h"
#include "rpi.h"
#include <limits.h>

accel_t mpu_accel_config(uint8_t addr, uint8_t g) {
    uint8_t accel_g;
    switch (g) {
    case 2: accel_g = 0b00; break;
    case 4: accel_g = 0b01; break;
    case 8: accel_g = 0b10; break;
    case 16: accel_g = 0b11; break;
    default:
        panic("mpu6050: invalid g value %d\n", g);
    };
    imu_wr(addr, ACCEL_CONFIG, accel_g << 3);

    printk("mpu6050 accel: configured to %d g\n", g);    

    return (accel_t) {
        .addr = addr,
        .g = g,
        .hz = 20
    };
};

triple_t mpu_accel_read_inner(accel_t *h) {
    while (!mpu_has_data(h -> addr))
        ;

    uint8_t data[6];
    imu_rd_n(h -> addr, ACCEL_XOUT, data, 6);

    int16_t x = (data[0] << 8) | data[1];
    int16_t y = (data[2] << 8) | data[3];
    int16_t z = (data[4] << 8) | data[5];

    return make_triple(x, y, z);
}

static int mg_scaled(int v, int g) {
    return (v * 1000 * g) / SHRT_MAX;
}

triple_t mpu_accel_scale(accel_t *h, triple_t xyz) {
    int g = h->g;
    int x = mg_scaled(h->g, xyz.x);
    int y = mg_scaled(h->g, xyz.y);
    int z = mg_scaled(h->g, xyz.z);
    return make_triple(x,y,z);
}

triple_t mpu_accel_read(accel_t *h) {
    return mpu_accel_scale(h, mpu_accel_read_inner(h));
}

