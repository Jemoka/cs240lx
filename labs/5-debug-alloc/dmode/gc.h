#ifndef __GC__
#define __GC__

extern void mark();
extern void mark_inner(uint32_t *sp);
void sweep();
void gc();

#endif

