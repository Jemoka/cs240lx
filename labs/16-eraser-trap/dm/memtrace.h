#ifndef __memtrace_h__
#define __memtrace_h__

#include "switchto.h"

#define MEMTRACE_FAULT_MAX 1000

typedef struct {
    regs_t regs_pre;
    regs_t regs_post;
    uint32_t pc;
    uint32_t addr;
    unsigned load_p:1;      // access = load (=1), or store (=0).
} memtrace_event;

typedef int (*memtrace_fn_t)(void *context, memtrace_event *e);

void memtrace_init(void *context, memtrace_fn_t pre_h, memtrace_fn_t post_h, unsigned trap_dom); 
void memtrace_start(); 
void memtrace_stop(); 
int memtrace_on_p();


#endif
