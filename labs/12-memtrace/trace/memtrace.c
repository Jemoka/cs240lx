#include "rpi.h"
#include "pinned-vm.h"
#include "full-except.h"
#include "armv6-except.h"
#include "switchto.h"
#include "watchpoint.h"
#include "memtrace.h"
#include "memmap-default.h"

static memtrace_t* memtrace_buf;

enum { 
    kern_dom = 1,
    heap_dom = 2,
    trap_heap_access = DOM_client << (kern_dom*2),
    no_trap          = trap_heap_access 
                     |  DOM_client << (heap_dom*2)
};

static int vm_map_everything(void) {
    // initialize the hardware MMU for pinned vm
    pin_mmu_init(no_trap);
    assert(!mmu_is_enabled());


    // compute the different mapping attributes.  
    // we only do simple uncached mappings today
    // (but shouldn't matter).

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    // we use 16mb section.
    pin_t dev  = pin_16mb(pin_mk_global(kern_dom, no_user, MEM_device));

    // kernel memory: same as device, but is only uncached.  
    pin_t kern = pin_mk_global(kern_dom, no_user, MEM_uncached);

    // heap.  different from kernel memory b/c:
    // 1. needs a different domain so will trap.
    // 2. user_access: since when we add single stepping 
    //    the code will run at user level.  (alternatively
    //    we could set <heap_dom> to manager permission)
    pin_t heap = pin_mk_global(heap_dom, user_access, MEM_uncached);

    // now identity map kernel memory.
    unsigned idx = 0;
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);

    // we could mess with the alignment to give the
    // heap more memory.
    pin_mmu_sec(idx++, SEG_HEAP, SEG_HEAP, heap);
    pin_mmu_sec(idx++, SEG_STACK, SEG_STACK, kern);
    pin_mmu_sec(idx++, SEG_INT_STACK, SEG_INT_STACK, kern);
    pin_mmu_sec(idx++, SEG_BCM_0, SEG_BCM_0, dev);

    // we aren't using user processes or anythings so we
    // just claim ASID=1 as our address space identifier.
    enum { ASID = 1 };
    pin_set_context(ASID);

    // turn the MMU on.
    assert(!mmu_is_enabled());
    mmu_enable();
    assert(mmu_is_enabled());
    // vm is now live!

    // return index in case if want to allocate more.
    return idx;
}

static void data_fault(regs_t *r) {
    // b4-43 [140e pinned mem]
    uint32_t reason     = data_abort_reason();
    // b4-44 [140e pinned mem]
    uint32_t fault_addr = data_abort_addr();

    // b4-20 has the different reasons.
    if(reason == DOMAIN_SECTION_FAULT) {
        memtrace_stop();
        // cheeky: works because pinned is mirrored is
        // identity mapped.
        watchpt_on_ptr((void *) fault_addr);
        // write down pre regs
        memtrace_event *e = &memtrace_buf->events[memtrace_buf->n];
        e->regs_pre = *r;
        e->pc = r->regs[15];
        e->addr = fault_addr;

        // decode *pc to figure out if load/store and record that too.
        uint32_t instr = get32((void *) e->pc);
        e->load_p = (instr & (1 << 20)) != 0;
    } else if (watchpt_fault_p()) {
        watchpt_off(fault_addr);
        // write down post regs
        memtrace_event *e = &memtrace_buf->events[memtrace_buf->n];
        memtrace_buf->n = (memtrace_buf->n + 1) % MEMTRACE_FAULT_MAX;
        e->regs_post = *r;

        /* uint32_t pc = watchpt_fault_pc(); */
        /* if(pc == (uint32_t)PUT32) { */
        /*     /\* trace("PUT32 fault at %x\n", fault_addr); *\/ */
        /* } else if(pc == (uint32_t)GET32) { */
        /*     trace("GET32 fault at %x\n", fault_addr); */
        /* } */
        memtrace_start();
    }

    /* trap_on(); */
    switchto(r);
}

void memtrace_init(memtrace_t *buf) {
    memtrace_buf = buf;
    memtrace_buf->n = 0;
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));

    // setup the full fault handlers [140e] that take in
    // the full register structure --- all 16 general
    // registers and the cpsr  --- that were live at the
    // fault.
    full_except_install(0);
    full_except_set_data_abort(data_fault);

    // map everything: when this returns vm is on!
    int idx = vm_map_everything();
    assert(mmu_is_enabled());

    let x = domain_access_ctrl_get();
    memtrace_stop();
}

void memtrace_start() {
    domain_access_ctrl_set(trap_heap_access);
    uint32_t v = domain_access_ctrl_get();
    assert(v = trap_heap_access);
}
void memtrace_stop() {
    domain_access_ctrl_set(no_trap);
    uint32_t v = domain_access_ctrl_get();
    assert(v = no_trap);
}

/* void notmain(void) { */
/*     kmalloc_init_set_start((void*)SEG_HEAP, MB(1)); */

/*     // setup the full fault handlers [140e] that take in */
/*     // the full register structure --- all 16 general */
/*     // registers and the cpsr  --- that were live at the */
/*     // fault. */
/*     full_except_install(0); */
/*     full_except_set_data_abort(data_fault); */

/*     // map everything: when this returns vm is on! */
/*     int idx = vm_map_everything(); */
/*     assert(mmu_is_enabled()); */

/*     // get the current domain. */
/*     let x = domain_access_ctrl_get(); */
/*     trace("%d total mappings, domain = %b\n", idx, x); */

/*     // make sure trapping is off while we mess with the */
/*     // heap. */
/*     trap_off(); */

/*     uint32_t *v = kmalloc(sizeof *v); */

/*     // turn trapping back on.  NOTE: for this */
/*     // simplistic test when trapping is on */
/*     // we can *only* read/write to heap memory */
/*     // using GET32/PUT32 b/c of how we wrote */
/*     // the data abort handler. */
/*     trap_on(); */

/*     // putting */
/*     trace("putting 64 at %x\n", v); */
/*     put32(v, 64); */
/*     uint32_t got = get32(v); */
/*     trace("got %d\n", got); */
/* } */
