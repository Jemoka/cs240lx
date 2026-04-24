#ifndef __mpu_6050_h__
#define __mpu_6050_h__

#include "rpi.h"
#include "i2c.h"
#include "mpu-regs.h"

// a tuple type
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} triple_t;
static inline triple_t make_triple(int16_t x, int16_t y, int16_t z) {
    triple_t t = {x, y, z};
    return t;
}

// read a single device register <reg> from i2c device 
// <addr> and return the result.
uint8_t imu_rd(uint8_t addr, uint8_t reg);

// set a single device register <reg> on device
// <addr> to value <v>
// 
// the operation is sent over i2c as two 8-bit values: 
// (byte 0 = <reg>, byte 1 = <v>)
void imu_wr(uint8_t addr, uint8_t reg, uint8_t v);

// do a "burst read" of <n> registers into buffer <v>, where 
//  - <addr> = device addr
//  - <base_reg> = lowest reg in sequence
//  - <n> = total number of 8-bit registers to read.
int imu_rd_n(uint8_t addr, uint8_t base_reg, uint8_t *v, uint32_t n);

// bootstrap the beginning, need to stick at the top of the driver
void mpu_init(uint8_t addr);  
// for resetting the device from original source
void mpu_reset(uint8_t addr);
// check if data is available on the fifo
int mpu_has_data(uint8_t addr);

// acceleration setup
typedef struct {
    uint8_t addr;
    uint8_t g; // accelerator resolution
    uint8_t hz;
} accel_t;
accel_t mpu_accel_config(uint8_t addr, uint8_t g);

// read a single tick from 
triple_t mpu_accel_read(accel_t *h);
triple_t mpu_accel_read_inner(accel_t *h);

// gyro setup
typedef struct {
    uint8_t addr;
    uint8_t g; // accelerator resolution
} gyro_t;
gyro_t mpu_gyro_config(uint8_t addr, uint16_t fss);
triple_t mpu_gyro_read(gyro_t *h);

// self test setup
typedef struct {
    triple_t a;
    triple_t g;
} selftest_t;
selftest_t mpu_measure_test(uint8_t addr);
void mpu_self_test_accel(accel_t *h);
void mpu_self_test_gyro(gyro_t *h);

#endif
