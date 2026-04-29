#include "rpi.h"
#include "rpi-math.h"

#include "mpu-6050.h"

double compute_ft(unsigned t) {
    if(t == 0)
        return 0;
    return 4096. * .34 * powf(.92/.34, (t-1.)/(2*2*2*2*2-2.));
}

double compute_gyro(unsigned t, int neg) {
    if (!neg)
        return 25. * 131. * powf(1.046, t - 1.);
    else    
        return -25. * 131. * powf(1.046, t - 1.);
}

selftest_t mpu_measure_test(uint8_t addr) {
    uint8_t data[4];
    imu_rd_n(addr, SELF_TEST_X, data, 4);
    delay_ms(100);

    // sorry I didn't use bit support lmao
    triple_t a = make_triple(((data[0] & 0b11100000) >> 3) | ((data[3] & 0b00000011)),
                             ((data[1] & 0b11100000) >> 3) | ((data[3] & 0b00001100) >> 2),
                             ((data[2] & 0b11100000) >> 3) | ((data[3] & 0b00110000) >> 4));
    triple_t g = make_triple(data[0] & 0b00011111, data[1] & 0b00011111, data[2] & 0b00011111);

    return (selftest_t) {
        .a = a,
        .g = g
    };
}

void mpu_self_test_accel(accel_t *h) {
    // turn on elf test
    uint8_t accel_config = 0b11100000 | (0b10 << 3);
    imu_wr(h->addr, ACCEL_CONFIG, accel_config);
    delay_ms(200);

    // self test measurement
    selftest_t test = mpu_measure_test(h->addr);

    // the test register
    double ft_ax = compute_ft(test.a.x);
    double ft_ay = compute_ft(test.a.y);
    double ft_az = compute_ft(test.a.z);
    // the reading
    triple_t accel_test = mpu_accel_read_inner(h);
    printk("mpu6050 accel reading: ax = %d, ay = %d, az = %d\n", accel_test.x, accel_test.y, accel_test.z);

    // disable self test
    imu_wr(h->addr, ACCEL_CONFIG, 0b10 << 3);
    delay_ms(200);

    // and then we turn tests off and read again
    mpu_measure_test(h->addr);
    triple_t accel_regular = mpu_accel_read_inner(h);
    printk("mpu6050 accel reading: ax = %d, ay = %d, az = %d\n", accel_regular.x, accel_regular.y, accel_regular.z);

    // ((enabled-disabled) - ft)/ft
    double ax = ((accel_test.x - accel_regular.x) - ft_ax) / ft_ax;
    double ay = ((accel_test.y - accel_regular.y) - ft_ay) / ft_ay;
    double az = ((accel_test.z - accel_regular.z) - ft_az) / ft_az;
    printk("mpu6050 accel self test: ax = %f, ay = %f, az = %f\n", ax*100, ay*100, az*100);
}

void mpu_self_test_gyro(gyro_t *h) {
    // turn on elf test
    uint8_t gyro_config = 0b11100000;
    imu_wr(h->addr, GYRO_CONFIG, gyro_config);
    delay_ms(200);

    // self test measurement
    selftest_t test = mpu_measure_test(h->addr);

    // the test register
    double ft_gx = compute_gyro(test.g.x, 0);
    double ft_gy = compute_gyro(test.g.y, 1);
    double ft_gz = compute_gyro(test.g.z, 0);
    // the reading
    triple_t gyro_test = mpu_gyro_read(h);
    printk("mpu6050 gyro reading: gx = %d, gy = %d, gz = %d\n", gyro_test.x, gyro_test.y, gyro_test.z);

    // disable self test
    imu_wr(h->addr, GYRO_CONFIG, 0);
    delay_ms(200);

    // and then we turn tests off and read again
    mpu_measure_test(h->addr);
    triple_t gyro_regular = mpu_gyro_read(h);
    printk("mpu6050 gyro reading: gx = %d, gy = %d, gz = %d\n", gyro_regular.x, gyro_regular.y, gyro_regular.z);

    // ((enabled-disabled) - ft)/ft
    double gx = ((gyro_test.x - gyro_regular.x) - ft_gx) / ft_gx;
    double gy = ((gyro_test.y - gyro_regular.y) - ft_gy) / ft_gy;
    double gz = ((gyro_test.z - gyro_regular.z) - ft_gz) / ft_gz;
    printk("mpu6050 gyro self test: gx = %f, gy = %f, gz = %f\n", gx*100, gy*100, gz*100);
}

