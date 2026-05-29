#include "rpi.h"
#include "pinned-vm.h"
#include "full-except.h"
#include "armv6-except.h"
#include "switchto.h"
#include "watchpoint.h"
#include "memtrace.h"
#include "memmap-default.h"

// the structure for storing trapped information
static void* memtrace_buf;

// the current event being handled
static memtrace_event event;

// domains for training and untrapping
static uint32_t dom_trap_map = 0;
static uint32_t dom_notrap_map = 0;

// trapping functions
static memtrace_fn_t pre;
static memtrace_fn_t post;

static void data_fault(regs_t *r) {
    // b4-43 [140e pinned mem]
    uint32_t reason     = data_abort_reason();
    // b4-44 [140e pinned mem]
    uint32_t fault_addr = data_abort_addr();

    // b4-20 has the different reasons.
    if(reason == DOMAIN_SECTION_FAULT) {
        memtrace_stop();
        // write down pre regs
        memtrace_event e;
        e.regs_pre = *r;
        e.pc = r->regs[15];
        e.addr = fault_addr;

        // decode *pc to figure out if load/store and record that too.
        uint32_t instr = get32((void *) e.pc);
        e.load_p = (instr & (1 << 20)) != 0;

        event = e;
        watchpt_on_ptr((void *) fault_addr);

        // call pre handler, if defined
        if (pre)
            pre(memtrace_buf, &event);
        // cheeky: works because pinned is mirrored is
        // identity mapped. otherwise need tlb?

    } else if (watchpt_fault_p()) {
        watchpt_off(fault_addr);
        event.regs_post = *r;

        // call post handler, if defined
        if (post)
            post(memtrace_buf, &event);


        memtrace_start();
    }

    switchto(r);
}

void memtrace_init(void *context, memtrace_fn_t pre_h, memtrace_fn_t post_h, unsigned trap_dom) {
    // stash pre and post handlers
    pre = pre_h;
    post = post_h;
    memtrace_buf = context;

    // and then set up trapping logistics
    // for the trap regime we set the trap domain to no access (00),
    // and the rest to client (0b01).
    dom_trap_map = ~(3 << (2*trap_dom));
    // for the non-trap domain, everybody be client (0b01)
    dom_notrap_map = 0x55555555;

    full_except_install(0);
    full_except_set_data_abort(data_fault);
}

void memtrace_start() {
    domain_access_ctrl_set(dom_trap_map);
}
void memtrace_stop() {
    domain_access_ctrl_set(dom_notrap_map);
}

int memtrace_on_p() {
    return (domain_access_ctrl_get() & dom_trap_map) == dom_trap_map;
}

