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

typedef struct {
    uint32_t n;
    memtrace_event events[MEMTRACE_FAULT_MAX];
} memtrace_t;

void memtrace_init(memtrace_t *buf); 
void memtrace_start(); 
void memtrace_stop(); 

#endif

