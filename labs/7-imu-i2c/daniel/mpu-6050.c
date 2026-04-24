#include "mpu-6050.h"
#include "mpu-regs.h"
#include "bit-support.h"
#include "rpi.h"

uint8_t imu_rd(uint8_t addr, uint8_t reg) {
    i2c_write(addr, &reg, 1);
    uint8_t v;
    i2c_read(addr,  &v, 1);
    return v;
}

int imu_rd_n(uint8_t addr, uint8_t base_reg, uint8_t *v, uint32_t n) {
    i2c_write(addr, (void*) &base_reg, 1);
    return i2c_read(addr, v, n);
}

void imu_wr(uint8_t addr, uint8_t reg, uint8_t v) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = v;
    i2c_write(addr, data, 2);
}

void mpu_init(uint8_t addr) {
    delay_ms(100);
    i2c_init();
    delay_ms(100);

    uint8_t whoami = imu_rd(addr, WHO_AM_I_REG);
    if (whoami != WHO_AM_I_VAL) {
        panic("mpu6050: expected %x, got %x\n", WHO_AM_I_VAL, whoami);
    }
    printk("mpu6050 initalized: whoami = 0x%x\n", whoami);
}

void mpu_reset(uint8_t addr) {
    // make sure device is up.
    delay_ms(100);

    // page 41: to reset device: set bit 7 = 1 in register
    // PWR_MGMT_1 (register 0x6b)
    imu_wr(addr, PWR_MGMT_1, bit_set(0, 7));

    // XXX: we should read different registers and see that they
    // went back to startup.

    // give time to shutdown, spin up.
    delay_ms(100);

    if(bit_is_on(imu_rd(addr, PWR_MGMT_1), 6))
        output("mpu6050 booted: device booted up in sleep mode!\n");

    // clear sleep mode: (PWR_MGMT_1)
    // if you do *NOT* do this, then the device we have does not work.
    // according to my reading of the data sheet, the value of 0x6b should
    // be 0 after reset.  so i don't get this.
    imu_wr(addr, PWR_MGMT_1, 0);

    delay_ms(100);

    // page 39: USER_CTRL (register 0x6a): reset:
    //   - the signal path = bit 0
    //   - i2c master mode = bit 1
    //   - fifo = bit 2
    // not sure if redundant after device reset --- datasheet
    // unclear --- so we do to be sure.
    imu_wr(addr, USER_CTRL, 0b111);

    delay_ms(100);

    // bit6: motion interrupt enable
    // NOTE: enable interrupts only on the IMU, not on your pi.
    // (INT_ENABLE) after you config (p27):
    // - latch to be held high until cleared; - bit 5
    // - read to clear it. - bit 4
    imu_wr(addr, INT_PIN_CFG, bit_set(0, 5) | bit_set(0, 4));
    imu_wr(addr, INT_ENABLE, bit_set(0, 0));
    output("mpu6050 booted: device boot finished!\n");
}

int mpu_has_data(uint8_t addr) {
    // check if DATA_RDY_INT is set in INT_STATUS (register 0x3a)
    return 0b1 & imu_rd(addr, INT_STATUS);
}

