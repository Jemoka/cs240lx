#include "rpi.h"
// base heap allocator
#include "kr-malloc.h"
// our tagging-based GC allocator
#include "gc-malloc.h"

GCHeader_t *alloclist;
static unsigned block_id=1;

// since, for gc, user may hand us memory in the middle of a block, we want to
// be able to check if a pointer is inside a block or not
unsigned pointer_inside_block_p(GCHeader_t *header, void *ptr) {
    char* start = (char *) header->self;
    char* end = (char *) header->self + header->size;
    return (char *)ptr >= start && (char *)ptr < end;
}

// walk the alloclist to see if the memory refers to a block we allocated
GCHeader_t *find_block(void *ptr) {
    for (GCHeader_t *cur = alloclist; cur != 0; cur = cur->next) {
        if (pointer_inside_block_p(cur, ptr)) {
            return cur;
        }
    }
    return 0;
}

GCHeader_t *filter(GCHeader_t *list, GCHeader_t *block) {
    if (list == 0) {
        return 0;
    }
    if (list == block) {
        return list->next;
    }
    list->next = filter(list->next, block);
    return list;
}

void gcfree(void *ptr) {

    // first, try to find the block that this pointer refers to
    GCHeader_t *block = find_block(ptr);
    demand(block != 0, "non-mananged pointer passed to gcfree: %x\n", ptr);
    demand(block->self == (GCHeader_t *) ptr, "pointer to middle of block passed to gcfree: %x\n", ptr);
    demand(block->state == ALLOC, "double free detected for pointer: %x\n", ptr);

    // mark block as free
    block->state = FREE;
    // remove this block from the alloclist so we don't find it again
    alloclist = filter(alloclist, block);
    // and then free the block back to the base allocator
    krfree(block);
}

void *gcmalloc(uint32_t nbytes) {
    // we want to allocate a block of memory that is big enough to hold the GC header and the data
    uint32_t total_size = sizeof(GCHeader_t) + nbytes;
    // we can just use our base allocator to get this memory
    GCHeader_t *block = (GCHeader_t *) krmalloc(total_size);
    if (block == 0) {
        return 0; // allocation failed
    }
    memset(block, 0, sizeof(GCHeader_t)); // zero out the block header for good measure

    // initialize the GC header
    block->size = nbytes;
    block->state = ALLOC;
    block->id = block_id++;
    // the data part of the block is right after the header, so we can set
    // self to point there for easy validation later
    block->self = block+1; 

    // add this block to the alloclist so we can find it later for GC
    block->next = alloclist;
    alloclist = block;

    // return a pointer to the data part of the block (not header)
    return (void *)(block + 1);
}

