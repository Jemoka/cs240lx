#include "mpu-6050.h"
#include "mpu-regs.h"
#include "bit-support.h"
#include "rpi.h"

gyro_t mpu_gyro_config(uint8_t addr, uint16_t fss) {
    uint8_t gyro_fss;
    switch (fss) {
    case 250: gyro_fss = 0b00; break;
    case 500: gyro_fss = 0b01; break;
    case 1000: gyro_fss = 0b10; break;
    case 2000: gyro_fss = 0b11; break;
    default:
        panic("mpu6050: invalid fss value %d\n", fss);
    };
    imu_wr(addr, GYRO_CONFIG, gyro_fss << 3);

    printk("mpu6050 gyro: configured to %d dps\n", fss);    

    return (gyro_t) {
        .addr = addr,
        .g = fss
    };
}

triple_t mpu_gyro_read(gyro_t *h) {
    while (!mpu_has_data(h -> addr))
        ;

    uint8_t data[6];
    imu_rd_n(h -> addr, GYRO_XOUT, data, 6);

    int16_t x = (data[0] << 8) | data[1];
    int16_t y = (data[2] << 8) | data[3];
    int16_t z = (data[4] << 8) | data[5];

    return make_triple(x, y, z);
}

    


