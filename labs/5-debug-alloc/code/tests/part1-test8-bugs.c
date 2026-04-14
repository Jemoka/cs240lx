// a bit more complicated:
//  1. do N random size allocations.
//  2. make sure there aren't errors.
//  3. do N random legal writes
//  4. make sure there aren't errors.
//  5. free everything
//  6. make sure there aren't errors.
//  7. do random illegal writes, record in <count>
//  8. make sure there are <count> errors.

// use after free.
#include "rpi.h"
#include "ckalloc.h"

struct list {
    struct list *next;
    unsigned v[1];
};

void notmain(void) {
    printk("test4: use after free\n");

    ck_verbose_set(0);
    enum { N  = 64 };

    //  1. do N random size allocations.
    struct list *l = 0;
    for(int i = 0; i < N; i++) {
        struct list * e = ckalloc(sizeof *e);
        e->next = l;
        l = e;
    }

    //  2. make sure there aren't errors.
    if(ck_heap_errors())
        panic("invalid error!!\n");
    else
        trace("SUCCESS heap checked out\n");

    //  3. do N random legal writes
    for(let p = l; p; p = p->next) 
        p->v[0] = 0;

    if(ck_heap_errors())
        panic("invalid error!!\n");
    else
        trace("SUCCESS heap checked out\n");

    //  5. free everything
    struct list *p,*e;
    for(p = l; p; p = e) {
        e = p->next;
        ckfree(p);
    }

    //  6. make sure there aren't errors.
    if(ck_heap_errors())
        panic("invalid error!!\n");


    uint32_t redzone32 = REDZONE_VAL << 24 
                       | REDZONE_VAL << 16
                       | REDZONE_VAL << 8
                       | REDZONE_VAL
                       ;
    //  7. do illegal writes
    l->v[0] = 1;
    if(ck_heap_errors())
        trace("found error\n");
    else
        panic("expected an error\n");

    // put it back.
    l->v[0] = redzone32;
    if(ck_heap_errors())
        panic("didn't reset?\n");

    l->v[1] = 1;
    if(ck_heap_errors())
        trace("found error\n");
    else
        panic("expected an error\n");
    trace("SUCCESS heap checked out\n");
}
