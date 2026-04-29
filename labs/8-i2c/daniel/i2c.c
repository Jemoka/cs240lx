#include "rpi.h"
#include "gpio.h"
#include "i2c.h"
#include "bit-support.h"


void i2c_init(i2c_t *g) {
    dev_barrier();

    // set pins to alt function 0
    gpio_set_function(g->scl, GPIO_FUNC_ALT0);
    gpio_set_function(g->sda, GPIO_FUNC_ALT0);
    dev_barrier();

    // clear fifo (1 on bit 5) and start (1 on bit 15)
    uint32_t c = bit_set(0, 5) | bit_set(0, 15);
    PUT32(BSC_C, c);

    // clear errors (1 on bit 8) and clear done (1 on bit 1)
    uint32_t s = bit_set(0, 9) | bit_set(0, 8) | bit_set(0, 1);
    PUT32(BSC_S, s);
    
    dev_barrier();
}
    
int i2c_active(void) {
    return bit_is_on(GET32(BSC_S), 0);
}

int i2c_readb(unsigned addr, uint8_t data[], unsigned nbytes) {
    while (i2c_active())
        ;
    
    // get the status and check for errors, if so return -1 and bail
    uint32_t s = GET32(BSC_S);
    uint32_t s_errorp = 0 |
        bit_is_on(s, 8) | // error
        bit_is_on(s, 9) | // clock stretch timeout
        bit_is_on(s, 5);  // fifo not empty (should be empty at this point)
    if (s_errorp) {
        /* printk("i2c: error detected before read: status = %x\n", s); */
        return -1;
    }
    s = bit_set(s, 1); // clear done bit
    PUT32(BSC_S, s);

    // set the device addr and number of bytes to read
    PUT32(BSC_A, addr);
    PUT32(BSC_DLEN, nbytes);

    // start read by setting bit 7
    // and set bit 0 to 1 for read (1 for write)
    // and set bit 5 to clear the fifo
    uint32_t c = bit_set(GET32(BSC_C), 7) | bit_set(0, 0) | bit_set(0, 5);
    PUT32(BSC_C, c);

    // block until we have data
    while (bit_is_off(GET32(BSC_S), 1))
        ;

    // and read!
    uint8_t *p = data;
    for (unsigned i = 0; i < nbytes; i++) {
        // block until we have data in the fifo
        while (bit_is_off(GET32(BSC_S), 5))
            ;
        /* printk("i2c: status = %x\n", GET32(BSC_S)); */
        // weeeee
        *p++ = GET32(BSC_FIFO) & 0xff;
    }

    // wait for done
    while (i2c_active())
        ;
    // error checking
    s = GET32(BSC_S);
    s_errorp = 0 |
        bit_is_on(s, 8) | // error
        bit_is_on(s, 9) | // clock stretch timeout
        bit_is_on(s, 0);  // transfer in progress (should be done at this point)
    if (s_errorp) {
        /* printk("i2c: error detected after read: status = %x\n", s); */
        return -1;
    }

    /* printk("i2c: READ DONE");     */

    delay_ms(100);
    return 0;
}

int i2c_writeb(unsigned addr, uint8_t data[], unsigned nbytes) {
    while (i2c_active())
        ;
    
    // get the status and check for errors, if so return -1 and bail
    uint32_t s = GET32(BSC_S);
    uint32_t s_errorp = 0 |
        bit_is_on(s, 8) | // error
        bit_is_on(s, 9) | // clock stretch timeout
        bit_is_off(s, 6);  // fifo not empty (should be empty at this point)
    if (s_errorp) {
        /* printk("i2c: error detected before write: status = %x\n", s); */
        return -1;
    }
    s = bit_set(s, 1); // clear done bit
    PUT32(BSC_S, s);

    // set the device addr and number of bytes to write
    PUT32(BSC_A, addr);
    PUT32(BSC_DLEN, nbytes);

    // start write by setting bit 7
    // and set bit 0 to 0 for write (1 for read)
    // and set bit 5 to clear the fifo
    uint32_t c = (bit_set(GET32(BSC_C), 7) | bit_set(0, 5)) & ~bit_set(0, 0);
    PUT32(BSC_C, c);

    // write the data into the fifo
    for (unsigned i = 0; i < nbytes; i++) {
        while (bit_is_off(GET32(BSC_S), 6))
            ;
        PUT32(BSC_FIFO, data[i]);
    }

    // wait for done
    while (i2c_active())
        ;
    
    // error checking
    s = GET32(BSC_S);
    s_errorp = 0 |
        bit_is_on(s, 8) | // error
        bit_is_on(s, 9) | // clock stretch timeout
        bit_is_on(s, 0);  // transfer in progress (should be done at this point)
    if (s_errorp) {
        /* printk("i2c: error detected after write: status = %x\n", s); */
        return -1;
    }

    /* printk("i2c: WRITE DONE");     */
    delay_ms(100);
    return 0;
}


