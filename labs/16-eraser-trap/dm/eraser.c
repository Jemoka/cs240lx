#include "prelude.h"

static eraser_state_t *shadow_memory;

static char *shadow_memory_start; // yes, this is just shadow_memory, but look ma no cast
static char *heap_start;

#define MAXTHREADS 16
static short lockset[MAXTHREADS];
static uint32_t cur_thread_id = 0;

eraser_state_t *eraser_shadow_addr(void *addr) {
    return (eraser_state_t *) (shadow_memory_start + ((char *) addr - heap_start));
}

static int eraser_handler(void *nil, memtrace_event *e) {
    uint32_t addr = e->addr;
    eraser_state_t *eras = eraser_shadow_addr((void *) addr);

    // two phases: we first transition the state diagram
    // and then we report errors **IF** we are in an error state
    switch (eras->state) {
    case SH_VIRGIN:
        // on write, transition to exclusive
        if (!e->load_p) {
            eras->tid = cur_thread_id;
            eras->state = SH_EXCLUSIVE;
        }
        break;
    case SH_EXCLUSIVE:
        // on first thread, we are fine
        if (eras->tid != cur_thread_id) {
            // a differet thread read it, we must now union the locksets
            eras->ls &= lockset[cur_thread_id];
            // on read, transition to shared
            if (e->load_p) {
                eras->state = SH_SHARED;
            } else {
                eras->state = SH_SHARED_MOD;
            }
        }
        break;
    case SH_SHARED:
        // since we are shared, anything taint us
        eras->ls &= lockset[cur_thread_id];
        if (!e->load_p) {
            eras->state = SH_SHARED_MOD;
        }
        break;
    case SH_SHARED_MOD:
        // since we are shared, anything taint us
        eras->ls &= lockset[cur_thread_id];
    }

    // we only report errors if we are in an error state
    if (eras->state == SH_SHARED_MOD) {
        // if the lockset is empty, we complain
        if (eras->ls == 0) {
            trace("BAD: tainted lockset on %p; owner %d\n", addr, eras->tid);
        }
    }

    return 0;
}

void lock(void *lockobj) {
    // JANK: we call the lockobj address the lockset
    lockset[cur_thread_id] |= (1 << ((uint32_t) lockobj % 16));
}
void unlock(void *lockobj) {
    // JANK: we call the lockobj address the lockset
    lockset[cur_thread_id] &= ~(1 << ((uint32_t) lockobj % 16));
}

void eraser_set_thread_id(char id) {
    cur_thread_id = id;
}

void eraser_onalloc(void *addr) {
    eraser_state_t *state = eraser_shadow_addr(addr);
    state->state = SH_VIRGIN;
    state->tid = cur_thread_id;
    state->ls = 0xffff; // all locks are free
}
void eraser_onfree(void *addr) {
    eraser_state_t *state = eraser_shadow_addr(addr);
    state->state = SH_FREED;
    state->ls = 0xffff;
}

void eraser_init(void) {
    // for initializing the heap
    sbrk_init();

    // initialize the shadow memory
    uint32_t heap_size = MB(1) / 2;
    shadow_memory = (eraser_state_t *) krmalloc(heap_size);
    shadow_memory_start = (char *) shadow_memory;
    heap_start = krmalloc(0); // jank: we figure out the start of the remaniing heap by

    // start trapping
    memtrace_init(0, eraser_handler, 0, dom_trap);
    memtrace_start();
}

char eraser_get_state(void *addr) {
    eraser_state_t *state = eraser_shadow_addr(addr);
    return state->state;
}

