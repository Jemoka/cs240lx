#ifndef __KR_MALLOC_H__
#define __KR_MALLOC_H__

void *sbrk(long increment);

union align {
    double d;
    void *p;
    void (*fp)(void);
};

typedef union header { /* block header */
    struct {
        union header *ptr; /* next block if on free list */
        unsigned size; /* size of this block */
    } s;
    union align x; /* force alignment of blocks */
} Header;

static Header *krmore(unsigned nunits);
void *krmalloc(unsigned nbytes);
void krfree(void *ptr);

#endif
