#include "kr-malloc.h"
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))
#define MIN_BLOCKS_PER_REGION 1024

static Header *freelist;
// A "degenerate" free list element: size 0, points to
// itself. This helps keep the freelist homogeneous.
static Header freelist_sentinel;

void krfree(void *ptr) {
    Header *block = (Header *)ptr - 1; // get the header of the block being freed

    // we want to figure out this block's parent
    Header *parent;
    for (parent = freelist; !(block > parent && block < parent->s.ptr); parent = parent->s.ptr) {
        // if we wrapped around the free list, then we want to stop
        // this means that the block we're trying to free is either before the
        // first block in the free list, or after the last block in the free list,
        // so we can just insert it at the end of the free list
        if (parent >= parent->s.ptr && (block > parent || block < parent->s.ptr)) {
            break;
        }
    }

    // join this block to the parent
    if (block + block->s.size == parent->s.ptr) {
        // this block is adjacent to the next block in the free list, so we can merge them
        block->s.size += parent->s.ptr->s.size;
        block->s.ptr = parent->s.ptr->s.ptr;
    } else {
        // this block is not adjacent to the next block in the free list, so we
        // just point it to the next block
        block->s.ptr = parent->s.ptr;
    }

    // join the parent to this block
    if (parent + parent->s.size == block) {
        // this block is adjacent to the parent block in the free list, so we can merge
        parent->s.size += block->s.size;
        parent->s.ptr = block->s.ptr;
    } else {
        // this block is not adjacent to the parent block in the free list, so we
        // just point it to this block
        parent->s.ptr = block;
    }

    freelist = parent; // update the start of the free list to the last not used candidate
}

// get the header to a new block of memory!
static Header *krmore(unsigned nunits) {
    if (nunits < MIN_BLOCKS_PER_REGION) {
        nunits = MIN_BLOCKS_PER_REGION;
    }

    char *new_region = sbrk(nunits * sizeof(Header));
    if (new_region == (char *) -1) {
        // no more memory!
        return 0;
    }

    Header *block = (Header *) new_region;
    block->s.size = nunits; // set the size of the new block
    krfree((void *)(block + 1)); // free the new block to add it to the free list
    return freelist; // return the start of the free list, which should now include the new block
}  

void *krmalloc(unsigned nbytes) {
    // number of blocks we need, including the header
    // this is computed by rounding up nbytes to a multiple of the block size,
    // and then adding one more block for the header  
    unsigned num_blocks = (nbytes + sizeof(Header)-1)/sizeof(Header) + 1;

    Header *block, *prev_block;

    // case: we don't have a free list yet, so let's make one
    // and point it to the generate case. we start to search
    // the freelist at *freelist     
    if ((prev_block = freelist) == 0) {
        // degenerate free list, see above
        freelist_sentinel.s.size = 0;
        freelist_sentinel.s.ptr = &freelist_sentinel;

        // set our freelist to point to here
        prev_block = freelist = &freelist_sentinel;
    }

    // walk our free list until we have a thing of the right size
    // or if we wrapped around then we ask the higher level allocator
    //
    // begin the walk of the freelist at the start; notice that we
    // can skip the first (sentinal) element    
    block = prev_block->s.ptr;
    while (1) {
        // this is big enough, so we can just use it
        if (block->s.size >= num_blocks) {
            if (block->s.size == num_blocks) {
                // exact fit, so we just remove this block from the free list
                prev_block->s.ptr = block->s.ptr;
            } else {
                // not an exact fit, so we split the block into two pieces:
                // one piece to return, and one piece to put back on the free list
                block->s.size -= num_blocks;
                block += block->s.size; // move pointer to the end of the remaining free block
                block->s.size = num_blocks; // set size of the allocated block
            }
            freelist = prev_block; // update the start of the free list to the last not used candidate
            // we will move the freelist back only when free happens, so for now
            // we can skip searching over ill-fitting blocks in the free list and

            return (void *)(block + 1); // return a pointer to the data part of the block (not header)
        }

        if (block == freelist) {
            // we walked all the way around the free list and didn't find anything
            // big enough, so we need to ask the higher level allocator for more memory
            block = krmore(num_blocks);
            if (block == 0) {
                return (void *) 0;
            }
        }

        // evidently this curently selection isn't big enough
        // so we increment to the next choice of free block
        prev_block = block;
        block = block->s.ptr;
    }

    return (void *) 0;
}


