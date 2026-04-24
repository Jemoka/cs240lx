#include "rpi.h"

void notmain(void) {
    enum { in = 20, out = 21 };

    gpio_set_output(out);
    gpio_set_input(in);
    
    enum { N = 5 };
    gpio_set_pulldown(out);

    // just to the boring expected loopback.
    output("about to do old-fashioned loopback\n");
    for(int i = 0; i < N; i++) {
        gpio_write(out, 1);
        if(!gpio_read(in))
            panic("add jumper from pin %d to pin %d\n", in,out);
        output("%d: good: wrote 1, read 1\n", i);

        gpio_write(out, 0);
        if(gpio_read(in))
            panic("add jumper from pin %d to pin %d\n", in,out);
        output("%d: good: wrote 0, read 0\n", i);
    }
    output("SUCCESS: loopback\n");


    // now do i2c style open drain
    output("about to do open-drain loopback\n");
    gpio_set_pullup(out);
    for(int i = 0; i < N; i++) {
        // write 1
        gpio_set_input(out);
        delay_us(1);  // pull this thread for fun :)
        if(!gpio_read(in))
            panic("add jumper from pin %d to pin %d\n", in,out);
        output("%d: good: wrote 1, read 1\n", i);

        // write 0
        gpio_set_output(out);
        gpio_write(out, 0);

        if(gpio_read(in))
            panic("add jumper from pin %d to pin %d\n", in,out);
        output("%d: good: wrote 0, read 0\n", i);
    }
    output("SUCCESS: loopback\n");
}
