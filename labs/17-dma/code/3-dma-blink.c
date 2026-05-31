// Same as 1-dma-blink, except now the `delay_ms` is also DMA
#include "rpi.h"
#include "dma-impl.h"

enum { GPIO_BASE  = 0x20200000 };

#define KB(x) ((x)*1024)

void gpio_set_on_dma(dma_ch_t *dma, unsigned pin) {
    uint8_t p = (0b1 << pin);
    cb_t a = cb_mk(bus((uint32_t *) (GPIO_BASE + 0x1c)), bus(&p), 1);
    dma_run(dma, &a, 100);
    /* todo("do a dma write to SET0\n"); */
}

void gpio_set_off_dma(dma_ch_t *dma, unsigned pin) {
    uint8_t p = (0b1 << pin);
    cb_t a = cb_mk(bus((uint32_t *) (GPIO_BASE + 0x28)), bus(&p), 1);
    dma_run(dma, &a,  100);

    /* todo("do a dma write to CLR0\n"); */
}

void dma_delay_ms(dma_ch_t *dma, unsigned ms) {
    uint32_t offset = 1000000;
    static volatile uint8_t one = 1;

    uint32_t cycle_count_start = timer_get_usec();
    cb_t a = cb_mk(bus(&one), bus(&one), KB(offset));
    /* a.TI |= (0x1f << WAITS_OFFSET); */
    a.TI = 0;
    dma_run(dma, &a, 1000000000);
    uint32_t cycle_count_stop = timer_get_usec();

    printk("delay_ms: requested %d ms, actual %d ms\n", ms, (cycle_count_stop - cycle_count_start)/1000);
}

void notmain(void) {
    // parthiv board led
    enum { pin = 27 };
    gpio_set_output(pin);

    enum { N = 4 };

#if 0
    // if you want to test with normal GPIO to make sure
    // works.
    output("going to blink parthiv board\n");
    for(int i = 0; i < N; i++) {
        gpio_set_on(pin);
        delay_ms(1000);
        gpio_set_off(pin);
        delay_ms(1000);
    }
#endif

    enum { dma_ch = 4 };
    let dma = dma_init(dma_ch);

    output("going to blink with DMA\n");
    for(int i = 0; i < N; i++) {
        gpio_set_on_dma(dma, pin);
        dma_delay_ms(dma, 1000);
        gpio_set_off_dma(dma, pin);
        dma_delay_ms(dma, 1000);
    }
}
