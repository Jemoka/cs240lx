#include "rpi.h"
#include "memtrace.h"

#include "watchpoint.h"

#include "mmu.h"
// 140e exception handling support
#include "full-except.h"
// 140e helpers for getting exception reason.
#include "armv6-except.h"
// 140e code for full context switching
// (caller,callee and cpsr).
#include "switchto.h"

#include "sbrk-trap.h"

// 1 = we expect a domain fault.
// 0 = we expect a or a watchpoint fault.
// used to catch some mistakes.
static int expect_domain_fault_p = 1;

// right now we only allow a single checker.  wrap this
// up for multiple checkers.
static memtrace_fn_t pre;
static memtrace_fn_t post;
static void *data;

static int quiet_p = 0;
void memtrace_yap_off(void) { quiet_p = 1; }
void memtrace_yap_on(void)  { quiet_p = 0; }

// pre-computed domain register values.
static uint32_t trap_access = 0;
static uint32_t no_trap_access = 0;

static int trap_is_on_p(void) {
    uint32_t val = domain_access_ctrl_get();
    return (val & trap_access) != trap_access;
}
static void trap_on(void) {
    domain_access_ctrl_set(trap_access);
}
static void trap_off(void) {
    domain_access_ctrl_set(no_trap_access);
}

// turn memtracing on: wrapper with extra error checking.
void memtrace_trap_enable(void) {
    // need at least one handler!
    assert(pre || post);
    // if not true, didn't init
    assert(trap_access && no_trap_access);
    assert(!trap_is_on_p());
    trap_on();
}

// turn memtracing off: wrapper with extra error checking.
void memtrace_trap_disable(void) {
    // if not true, didn't init
    assert(trap_access && no_trap_access);
    assert(trap_is_on_p());
    trap_off();
}

// XXX: a good extension: change this so you look at the
// actual instruction and get the actual bytes.
static inline unsigned inst_nbytes(uint32_t inst) {
    return 4;
}

static void data_fault(regs_t *r) {
    // b4-43 [140e pinned mem]
    uint32_t reason     = data_abort_reason();
    // b4-44 [140e pinned mem]
    uint32_t fault_addr = data_abort_addr();


    // sanity check that we still at SUPER
    //   - should make it so we can run at user level.
    if(mode_get(r->regs[16]) != SUPER_MODE)
        panic("got a fault not at SUPER level?\n");


    if(reason == DOMAIN_SECTION_FAULT) {
        trap_off();
        // cheeky: works because pinned is mirrored is
        // identity mapped.
        watchpt_on_ptr((void *) fault_addr);

        uint32_t instr = get32((void *) r->regs[15]);
        fault_ctx_t ctx = fault_ctx_mk(r, fault_addr, 4, (instr & (1 << 20)) != 0);
        if(pre)
            pre(data, &ctx);

        // decode *pc to figure out if load/store and record that too.
        uint32_t instr = get32((void *) e->pc);
        e->load_p = (instr & (1 << 20)) != 0;
    } else if (watchpt_fault_p()) {
        watchpt_off(fault_addr);


        *((unsigned*) data) += 1;


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

    // after a domain fault: call <pre>.  
    // after a watchpoint fault: call <post>.
    /* todo("handle the fault!"); */

    // drain printk to avoid the "can tx" race in UART.
    while(!uart_can_put8())
        ;

    switchto(r);
}

// initialize memtrace system.
void memtrace_init(
    void *data_h,
    memtrace_fn_t pre_h,
    memtrace_fn_t post_h,
    unsigned trap_dom) {

    // setting up VM does not belong here, but we do it to keep things
    // simple for today's lab.
    assert(!mmu_is_enabled());
    sbrk_init();
    assert(mmu_is_enabled());

    pre = pre_h;
    post = post_h;
    if(!pre && !post)
        panic("must supply one handler: pre=%x, post=%x\n", pre,post);
    data = data_h;
    assert(trap_dom < 16);

    // set up trap domains access registers
    /* The purpose of the fields D15-D0 in the register is to define the access permissions for each one of */
    /* the 16 domains. These domains can be either sections, large pages or small pages of memory: */
    /* b00 = No access, reset value. Any access generates a domain fault. */
    /* b01 = Client. Accesses are checked against the access permission bits in the TLB entry. */
    /* b10 = Reserved. Any access generates a domain fault. */
    /* b11 = Manager. Accesses are not checked against the access permission bits in the TLB entry, so a */
    /* permission fault cannot be generated. */
    // for the trap regime
    // we set the trap domain to no access, and the rest to client.
    trap_access = ~(3 << (2*trap_dom));
    
    // for the non-trap regime, we set all domains to client.
    no_trap_access = 0x55555555;

    // setup checkers
    pre = pre_h;
    post = post_h;
    // and data
    data = data_h;
    

    /* todo("do any additional setup you need"); */

    // XXX: what's the right way to handle SS exceptions at the same time?
    full_except_install(0);
    full_except_set_data_abort(data_fault);
}
