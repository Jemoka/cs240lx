#ifndef __i2c_h__
#define __i2c_h__

#define BSC_BASE 0x20804000

enum {
    BSC_C = BSC_BASE + 0x00,
    BSC_S = BSC_BASE + 0x04,
    BSC_DLEN = BSC_BASE + 0x08,
    BSC_A = BSC_BASE + 0x0c,
    BSC_FIFO = BSC_BASE + 0x10,
    BSC_DIV = BSC_BASE + 0x14,
    BSC_DEL = BSC_BASE + 0x18,
    BSC_CLKT = BSC_BASE + 0x1c
};

typedef struct {
    uint8_t scl;
    uint8_t sda;
} i2c_t;  

void i2c_init(i2c_t *g);
int i2c_writeb(unsigned addr, uint8_t data[], unsigned nbytes);
int i2c_readb(unsigned addr, uint8_t data[], unsigned nbytes);

#endif

