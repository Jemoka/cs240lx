#ifndef __GC_MALLOC__
#define __GC_MALLOC__

enum {
    ALLOC = 11,
    FREE
} GCMode;

typedef struct GCHeader {
    struct GCHeader *next;
    // for pointer validation, keep track of the
    // supposed location of this block
    struct GCHeader *self;
    // where the next block is (free or not)
    size_t size;
    // whether this block is marked or not (for GC)
    // either ALLOC or FREE
    uint32_t state;
    // mark, for mark-and-sweep GC
    uint16_t marked;
    // and an id for debugging purposes
    uint32_t id;
} GCHeader_t;

GCHeader_t *find_block(void *ptr);
unsigned pointer_inside_block_p(GCHeader_t *header, void *ptr);

void *gcmalloc(uint32_t nbytes);
void gcfree(void *ptr);

#endif

