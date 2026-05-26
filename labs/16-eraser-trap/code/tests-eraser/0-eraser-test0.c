// engler: doesn't do anything interesting: just checks translation 
// and mmu on/off as expected.
#include "eraser.h"

#include "mmu.h"

int x = 0x12345678;

void notmain(void) {
    trace("check that we are running at in superuser mode\n");

    assert(!mmu_is_enabled());
    eraser_init();
    eraser_verbose_set(1);

    // set the current thread id.
    eraser_set_thread_id(1);

    assert(mmu_is_enabled());
    assert(mode_eq(SUPER_MODE));
    //make sure translation works.
    assert(x == 0x12345678);

    // NOTE: naive-eraser will flag if we access
    // the heap without a lock.   
    //
    // to sanity test the basic operations
    // we do this in a weird way: access the
    // heap by turning trapping off (so eraser
    // doesn't flag that we have no lock), then
    // turn it back on.
    
    // int *y = eraser_alloc(4);
    memtrace_trap_disable();
    int *y = kmalloc(4);
    memtrace_trap_enable();

    memtrace_trap_disable();
    *y = 1;
    assert(*y == 1);
    memtrace_trap_enable();


    memtrace_trap_disable();
    // MMU still on, just don't trap.  [not that intuitive]
    assert(mmu_is_enabled());

    trace("SUCCESS: asserts passed\n");
}
