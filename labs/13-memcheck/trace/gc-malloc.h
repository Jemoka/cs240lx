#ifndef __GC_MALLOC__
#define __GC_MALLOC__

#define GC_FIFO_SIZE 128

#define REDZONE_VALUE 0xfe
#define REDZONE_SIZE 128

typedef uint8_t GCRedzone_t[128];

typedef enum {
    ALLOC = 11,
    FREE
} GCMode;

typedef struct GCHeader {
    struct GCHeader *next;
    // for pointer validation, keep track of the
    // supposed location of this block
    struct GCHeader *self;
    // where the next block is (free or not)
    uint32_t size;
    // whether this block is marked or not (for GC)
    // either ALLOC or FREE
    uint32_t state;
    // mark, for mark-and-sweep GC
    uint32_t marked;
    // and an id for debugging purposes
    uint32_t id;
    // redzone to detect buffer overflows
    GCRedzone_t redzone_front;
} GCHeader_t;

uint32_t check_heap();
GCHeader_t *find_block(void *ptr);
GCHeader_t *find_block_everywhere(void *ptr);
uint32_t pointer_inside_block_p(GCHeader_t *header, void *ptr);

void *gcmalloc(uint32_t nbytes);
void gcfree(void *ptr);
int gcgood(void *ptr);
int gcdist(void *ptr);

#endif

