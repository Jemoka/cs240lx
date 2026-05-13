#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

extern GCHeader_t *alloclist;
extern uint32_t *__data_start__;
extern uint32_t *__data_end__;
extern uint32_t *__bss_start__;
extern uint32_t *__bss_end__;

// for a particular pointer, we check if it points to a block we allocated, and if so, we mark that block and recursively mark all the pointers inside that block
void mark_pointees(uint32_t *sp) {
    GCHeader_t *block = find_block(sp);
    if (block == 0) {
        // this pointer doesn't point to a block we allocated, so we can just ignore it
        return;
    }
    if (block->marked) {
        // this block is already marked, so we can just ignore it
        return;
    }
    // mark this block
    block->marked = 1;
    // and then recursively mark all the pointers inside this block
    uint32_t *data_start = (uint32_t *) (block->self + 1); // data starts right after the header
    uint32_t *data_end = (uint32_t *) ((char *) block->self + sizeof(GCHeader_t) + block->size); // data ends at the end of the block
    for (uint32_t *p = data_start; p < data_end; p++) {
        // for each pointer inside this block, we want to mark it as well; notice
        // that we need to unpack the pointer value from the data
        mark_pointees((uint32_t *) *p);
    }
}

extern void mark_inner(uint32_t *sp) {
    // mark stack and also the registers (we already pushed the registers onto the
    // stack in the assembly code, so we can just mark everything on the stack)
    uint32_t *stack_top = (void*) STACK_ADDR;
    for (uint32_t *p = sp; p < stack_top; p++) {
        mark_pointees((uint32_t *) *p);
    }

    // and mark data and bss sections
    for (uint32_t *p = __data_start__; p < __data_end__; p++) {
        mark_pointees((uint32_t *) *p);
    }
    for (uint32_t *p = __bss_start__; p < __bss_end__; p++) {
        mark_pointees((uint32_t *) *p);
    }
}

void sweep() {
    // walk the alloclist and free all the blocks that are not marked
    for (GCHeader_t *cur = alloclist; cur != 0; ) {
        if (!cur->marked && cur->state == ALLOC) {
            void *self = cur->self; // keep track of what to free
            cur = cur->next; // move to next block before freeing this one
            gcfree(self); // free this block
        } else {
            // this block is marked, so we want to unmark it for the next round of GC
            cur->marked = 0;
            cur = cur->next; // move to next block
        }
    }
}

void gc() {
    mark();
    sweep();  
}
