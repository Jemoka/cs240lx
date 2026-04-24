#ifndef __mpu_regs_h__
#define __mpu_regs_h__

enum {
    // setup logistics
    WHO_AM_I_REG = 0x75,
    WHO_AM_I_VAL = 0x68,

    // power and user management
    USER_CTRL = 0x6a,
    PWR_MGMT_1 = 0x6b,

    // interrupt shenanigans
    INT_PIN_CFG = 0x37,
    INT_ENABLE = 0x38,
    INT_STATUS = 0x3a,

    // accelerometer setup
    ACCEL_CONFIG = 0x1c,
    ACCEL_XOUT = 0x3b,

    // self test setup
    SELF_TEST_X = 0x0d,
    SELF_TEST_Y = 0x0e,
    SELF_TEST_Z = 0x0f,
    SELF_TEST_A = 0x10,

    // gyro setup
    GYRO_CONFIG = 0x1b,
    GYRO_XOUT = 0x43
};

#endif

