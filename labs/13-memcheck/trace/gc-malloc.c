#include "rpi.h"
// base heap allocator
#include "kr-malloc.h"
// our tagging-based GC allocator
#include "gc-malloc.h"

GCHeader_t *alloclist;
GCHeader_t *freelist[GC_FIFO_SIZE] = {0};
static uint32_t freelist_offset = 0;
static uint32_t block_id=1;

// round-robin free list management for FIFO behavior
void free_block(GCHeader_t *block) {
    uint32_t idx = freelist_offset++ % GC_FIFO_SIZE;
    // if there is already a block in this slot, free it back to the base allocator
    if (freelist[idx] != 0) {
        krfree(freelist[idx]);
    }
    freelist[idx] = block;
}

// since, for gc, user may hand us memory in the middle of a block, we want to
// be able to check if a pointer is inside a block or not
uint32_t pointer_inside_block_p(GCHeader_t *header, void *ptr) {
    char* start = (char *) header->self;
    char* end = (char *) header->self + header->size;
    return (char *)ptr >= start && (char *)ptr < end;
}

// this one is whether the pointer is in the entiretiy of the block
// front, back, redzone, etc.
uint32_t pointer_inside_whole_block_p(GCHeader_t *header, void *ptr) {
    char* start = (char *) header;
    char* end = (char *) header + sizeof(GCHeader_t) + header->size + sizeof(GCRedzone_t);
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

// walk the alloclist to see if the memory refers to a block we managed
// that is, anything in the alloclist, or anything in the freelist
GCHeader_t *find_block_everywhere(void *ptr) {
    for (GCHeader_t *cur = alloclist; cur != 0; cur = cur->next) {
        if (pointer_inside_whole_block_p(cur, ptr)) {
            return cur;
        }
    }
    for (uint32_t i = 0; i < GC_FIFO_SIZE; i++) {
        if (freelist[i] != 0) {
            if (pointer_inside_whole_block_p(freelist[i], ptr)) {
                return freelist[i];
            }
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

uint32_t check_block(GCHeader_t *block, int check_content) {
    // check that the redzones are intact
    GCRedzone_t *redzone_front = (GCRedzone_t *) block->redzone_front;
    GCRedzone_t *redzone_back = (GCRedzone_t *) ((char *) block + sizeof(GCHeader_t) + block->size);
    for (int i = 0; i < REDZONE_SIZE; i++) {
        if ((*redzone_front)[i] != REDZONE_VALUE) {
            printk("buffer underflow detected for block id: %d\n", block->id);
            return 1;
        }
        if ((*redzone_back)[i] != REDZONE_VALUE) {
            printk("buffer overflow detected for block id: %d\n", block->id);
            return 2;
        }
    }

    if (!check_content) {
        return 0;
    }

    // if we want to check content, we additionally check that the contents of
    // the block are the redzone value
    for (int *p = (int *) block->self; p < (int *) ((char *) block->self + block->size); p++) {
        if (*p != REDZONE_VALUE) {
            printk("use-after-free detected for block id: %d\n", block->id);
            return 3;
        }
    }

    return 0;
}

uint32_t check_heap() {
    uint32_t nerrors = 0;

    // first walk the allocated blocks and check that their redzones are intact
    for (GCHeader_t *cur = alloclist; cur != 0; ) {
        if (check_block(cur, 0) != 0) {
            nerrors++;
        }
        cur = cur->next;
    }

    // and walk the free blocks for use-after free    
    for (uint32_t i = 0; i < GC_FIFO_SIZE; i++) {
        if (freelist[i] != 0) {
            if (check_block(freelist[i], 1) != 0) {
                nerrors++;
            }
        }
    }

    return nerrors;
}

// returns 0 if ptr is a good pointer that should have data
// 1 if the pointer is mismanaged
// 2 if the pointer managed but weird
int gcgood(void *ptr) {
    // first, try to locate the block
    GCHeader_t *block = find_block_everywhere(ptr);
    if (block == 0) {
        return 1;
    }
    if (!find_block(ptr)) {
        return 2; // in redzone etc.
    }

    return 0;
}

// describe the offset:
// positive values---we are in the end redzone somewhere
// negative value---we are in the block header or front redzone somewhere
int gcdist(void *ptr)  {
    GCHeader_t *block = find_block_everywhere(ptr);
    if (find_block(ptr) == block) {
        // if the pointer is inside the block, then we return 0 since it's not an error
        return 0;
    }

    // check if the pointer is before the data
    if ((char *) ptr < (char *) block->self) {
        // if it's before the data, then we return the negative distance to the start of the data
        return (char *) ptr - (char *) block->self;
    }

    // and otherwise, we check distance in the back redzone
    return ((char *) ptr - ((char *) block->self + block->size))+1;
}

void gcfree(void *ptr) {
    // first, try to find the block that this pointer refers to
    GCHeader_t *block = find_block(ptr);


    demand(block != 0, "non-mananged pointer passed to gcfree: %x\n", ptr);
    demand(block->self == (GCHeader_t *) ptr, "pointer to middle of block passed to gcfree: %x\n", ptr);
    demand(block->state == ALLOC, "double free detected for pointer: %x\n", ptr);

    // go through the redzones and check that they are intact
    GCRedzone_t *redzone_front = (GCRedzone_t *) block->redzone_front;
    GCRedzone_t *redzone_back = (GCRedzone_t *) ((char *) block + sizeof(GCHeader_t) + block->size);
    if (check_block(block, 0) != 0) {
        // if the block is corrupted, we don't want to free it back to the base allocator since
        // that could cause further corruption, so we just return early
        demand(0, "block id %d is corrupted, not freeing back to base allocator\n", block->id);
    }
    for (int *p = (int *) block->self; p < (int *) ((char *) block->self + block->size); p++) {
        *p = REDZONE_VALUE; // so that we can detect use-after-free
    }

    // mark block as free
    block->state = FREE;
    // remove this block from the alloclist so we don't find it again
    alloclist = filter(alloclist, block);
    // and then free the block back to the base allocator
    free_block(block);
}

void *gcmalloc(uint32_t nbytes) {
    if (nbytes == 0) {
        printk("warning: gcmalloc called with size 0, returning null pointer\n");
        return 0;
    }
    // we want to allocate a block of memory that is big enough to hold the GC header and the data
    uint32_t total_size = sizeof(GCHeader_t) + nbytes + sizeof(GCRedzone_t); // add space for redzone at the end of the block
    // we can just use our base allocator to get this memory
    GCHeader_t *block = (GCHeader_t *) krmalloc(total_size);
    if (block == 0) {
        return 0; // allocation failed
    }
    memset(block, 0, sizeof(GCHeader_t)); // zero out the block header for good measure

    // take a pointer to the front and back redzones and fill them with the redzone value for overflow detection    
    GCRedzone_t *redzone_front = (GCRedzone_t *) block->redzone_front;
    GCRedzone_t *redzone_back = (GCRedzone_t *) ((char *) block + sizeof(GCHeader_t) + nbytes);
    for (unsigned i = 0; i < REDZONE_SIZE; i++) {
        (*redzone_front)[i] = REDZONE_VALUE;
        (*redzone_back)[i] = REDZONE_VALUE;
    }

    // initialize the GC header
    block->size = nbytes;
    block->state = ALLOC;
    block->id = block_id++;
    // the data part of the block is right after the header, so we can set
    // self to point there for easy validation later
    block->self = block+1;




    /* printk("gcmalloc: allocated block id %d at %x\n", block->id, block+1); */

    // add this block to the alloclist so we can find it later for GC
    block->next = alloclist;
    alloclist = block;

    // return a pointer to the data part of the block (not header)
    return (void *)(block + 1);
}

