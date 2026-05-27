#include "rpi.h"
#include "mpu-6050.h"

#define DIR 6
#define STEP 7

uint8_t ADDR = 0b1101000;

typedef struct {
    uint8_t dir;
    uint8_t step;
} stepper_t;


static stepper_t init(uint8_t dir, uint8_t step) {
    gpio_set_output(dir);
    gpio_set_output(step);

    return (stepper_t) {
        .dir = dir,
        .step = step
    };
}

static void step(stepper_t stepper) {
    gpio_set_on(stepper.step);
    delay_us(5);
    gpio_set_off(stepper.step);
    delay_us(5);
}

static void pulse(stepper_t stepper, uint32_t delay) {
    for (int i = 0; i < 100; i++) {
        step(stepper);
        delay_us(delay);
    }
}



void notmain(void) {

    mpu_init(ADDR);
    mpu_reset(ADDR);
    gyro_t gyro = mpu_gyro_config(ADDR, 250);
    stepper_t t = init(DIR, STEP);
    
    while (1) {
        triple_t g = mpu_gyro_read(&gyro);
        unsigned x = g.x > 0 ? g.x : -g.x;
        printk("hi? %d\n", x);
        pulse(t, x+1000);
        /* delay_ms(100); */
    }


    return;
}

/* #include "rpi.h" */
/* #include "mpu-6050.h" */

/* uint8_t ADDR = 0b1101000; */

/* #define dir 21 */
/* #define step 20 */

/* typedef struct { */
/*     uint8_t dir; */
/*     uint8_t step; */
/* } stepper_t; */

/* static stepper_t init(uint8_t dir, uint8_t step) { */
/*     gpio_set_output(dir); */
/*     gpio_set_output(step); */

/*     return (stepper_t) { */
/*         .dir = dir, */
/*         .step = step */
/*     }; */
/* } */

/* static void step(stepper_t stepper) { */
/*     gpio_set_on(stepper.step); */
/*     delay_us(5); */
/*     gpio_set_off(stepper.step); */
/*     delay_us(5); */
/* } */

/* static void pulse(stepper_t stepper, uint32_t delay) { */
/*     for (int i = 0; i < 100; i++) { */
/*         step(stepper); */
/*         delay_us(delay); */
/*     } */
/* } */

/* void notmain(void) { */
/*     gyro_t gyro = mpu_gyro_config(ADDR, 500); */
/*     while (1) { */
/*         triple_t g = mpu_gyro_read(&gyro); */
/*         printk("gyro: x = %d, y = %d, z = %d\n", g.x, g.y, g.z); */
/*         delay_ms(100); */
/*     } */
/*     stepper_t t = init(DIR, STEP); */

/*     pulse(t, 2048); */
/*     delay_ms(1000); */
/*     pulse(t, 7000); */
/*     pulse(t, 8000); */
/*     pulse(t, 1000); */
/*     delay_ms(10); */
/*     pulse(t, 3000); */
/* } */


