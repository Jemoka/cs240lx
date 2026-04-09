---

marp: true
theme: default
paginate: true
html: true                      

style: |
section ul,
section ol {
  line-height: 1.2;
  margin-top: 0.1em;
  margin-bottom: 0.1em;
}
section li + li {
    margin-top: 0.1em;
}


---
![bg right:55% contain](images/aspiration.jpg)

# Boehm garbage collection notes 
(240lx spr26)

---
## Today: continue LX method: break things apart ==> power.
 - labs 1/2: code = data.
   - You have been generating data since you could type.
   - You don't need a compiler to make code, you can generate it 
   - You don't need to do code statically: can do at runtime.
 - Today: GC
   - Raw memory is not opaque: its contents have meaning.
   - Rip it apart and can dynamically type.
   - Will use that to do garbage collection: automatic free.
 - Readings: two massive hacks:
   - Boehm GC paper
   - Purify: dynamically find memory leaks and corruption.


---
## Building garbage collection (GC) for C (Rust? Zig?)

- GC: automatically free unused memory (garbage)
- "Can you do garbage collection in C"
  - Everyone: "No.  That's stupid: you don't know the types."
  - Boehm: "Yes."
  - "Ah."
- Today will build it. Our bedrock machine for winning: 
   1. b/c all the code is ours, 
   2. and you understand it;
   3. can easily build high concept things in a few hours (that would take months on a general OS).
- Money-back Guarantee: by 10pm today you will think a bit differently.

---
## Basic intuitions.

- If no pointer points-to an allocated block B, B is *lost*.
- Leak detection using lost blocks:
  - IF: each `B = malloc()` must have a matching `free(B)`
  - THEN: any lost block B is a leak.  Flag it as an error.
- Garbage collection using lost blocks:
  - Since a lost block B cannot be reached, it can't be used.
  - Thus: the system can do `free(B)` --- code can't tell the difference!
- Requires:
  1. You know all allocated blocks.
  2. You can find pointers.  (Don't need all, but more = better.)

---
## How?   One method: mark-and-sweep

- Intution: garbage = not reachable with graph traversal from root-set.
  1. get the root-set.
  2. mark: do a graph traversal *marking* every reachable allocated block.
  3. linear sweep all allocated blocks:  not reachable from (2)?   garbage: free it.
```
    mark_and_sweep()
       # all other memory locations must be reachable
       # from these starting points.
       root_set = { valid-pointers in static data }
                U { valid-pointers in registers }
                U { valid-pointers on the stack };
        mark(root_set);
        sweep();
    }
```

---
## Mark-and-sweep

```
    mark(pointer_set) {
       foreach p in pointer_set {
            if !marked(p) {
                marked(p) = 1;
                mark({ valid pointers in *p })
            }
        }
    }
    sweep() {
        foreach blk in heap {
            # reachable
            if(marked(blk))
                marked(blk) = 0;
            # not reachable
            else
                free(blk);
        }
    }
```

---
## The key function: computing "valid pointers"

- C memory is just bytes.  No types.  No rules.
  - How do you find the valid pointers?

- Boehm hack: 
  - IF memory word's integer value = the address of allocated block B.
  - THEN: consider B = reachable.

- Finding all potential pointers on ARMv6
  - 32-bit addresses (i.e., ldr, str, ldm, stm: base register is 32-bits)
  - 4-byte alignment.
  - So: scan memory in units of 4-byte aligned 32-bit words.
  

# Why Conservative GC?

C gives us raw bytes, not pointer types.

So the README uses Boehm’s trick:
- treat any **aligned 4-byte word** as a potential pointer
- if its value falls inside an allocated block, mark that block reachable

---
## Simple example

```text
simple block:

B = malloc(16) at 0x10004000
B spans [0x10004000, 0x10004010)

r1    = 0x10004000  -> start pointer
stack = 0x10004008  -> middle pointer
heap  = 0x10004004  -> middle pointer found while scanning heap
```

Effect:
- all three can keep `B` marked
- start refs are stronger evidence than middle refs
- conservative GC may retain garbage, but avoids **freeing a live block**


---
## What you build

1. Port K&R `malloc/free` to the Pi
2. Add a `ckalloc` veneer that tracks blocks
3. Implement conservative mark-and-sweep leak detection
4. Reuse that machinery to reclaim unreachable blocks

Big theme:
- correctness first
- avoid clever allocator tricks until the invariants are solid
- Great for extensions!

---
# Part 1: Start With K&R `malloc`

- Copy and paste the allocator from `docs/kr-malloc.pdf`
- Make it compile and work
- Then adapt `sbrk()` on the Pi with `kmalloc`
- Goal: get a slow but trustworthy `free()`

Why start here?
- `free()` is where allocator state gets hard
- every later step assumes allocation metadata stays consistent

---
# Part 2: wrap it up with `ckalloc`

The GC needs two questions answered:
1. Which blocks are currently allocated?
2. If some word looks like a pointer, what block does it refer to?

How:
- prepend a header to every allocation
- keep all allocated blocks on a linked list


---
## Heap Layout Diagram

```text
memory from kr_malloc

+---------------- hdr_t ---------------+--------- payload ---------+
| next | nbytes | state | id | refs... |  bytes returned to user   |
+--------------------------------------+---------------------------+
^
|
metadata used by ckalloc and GC
```

Why this header helps:
- block size is explicit
- block id is stable no matter where laid out in memory.
- can take random integer and map to containing block (if any) by linear
  traversal.

---
## Allocated List Diagram

```text
alloc_list
   |
   v
[hdr C] ---> [hdr B] ---> [hdr A] ---> 0
   |           |            |
   v           v            v
 block C     block B      block A
```

Operationally:
- `ckalloc` inserts a new header into this list
- `ckfree` removes the header before freeing the block
- `ck_ptr_is_alloced(p)` can walk the list to resolve candidate pointers



---
## `ckalloc` Invariants

Two key checks:
- Before adding block `B`, `ck_ptr_is_alloced(B)` should fail
- After adding `B`, `ck_ptr_is_alloced(B)` should succeed
- Before `ckfree(B)`, `B` must still be on the allocated list
- After removing it, `ck_ptr_is_alloced(B)` should fail

These are small invariants, but they make the later GC code believable.


---
## Start Pointer Vs Middle Pointer

Header has two counters:
- `refs_start`
- `refs_middle`

```text
block:

start                                      end
  v                                         v
+---------------------------------------------+
| payload payload payload payload payload ... |
+---------------------------------------------+
  ^                  ^
  |                  |
start ref         middle ref
```

Why track both for leaks?
- a pointer to the **start** is stronger evidence of liveness
- a pointer only to the **middle** might be a real interior pointer
- or it might be a coincidental integer value

That is why the tool can report **definite** vs **maybe** leaks.

---
## A big source of WTF: kill a pointer, but no leak

- The C source might kill a pointer.  Your GC doesn't catch it
```
    p = malloc(8);
    p = 0;
    ck_check_leaks() ==> 0   
```
- What is going on??
- How to handle?
  - Need to make sure check after routine returns.
  - Isn't inlined.
  - Epsilon deduction: Keep cutting down to smallest failing test.


---
# Some more pitfalls

```text
easy sweep bug:

for (h = first; h; h = h->next) {
    if (need to free h)
        ckfree(h);   // but what is h->next now?
}
```

More pits to fall:
- compiler optimization can erase a pointer you expected to survive
- stale stack/register data can keep dead blocks looking reachable
- off-by-one scan ranges silently miss pointers
- tests should be checked at `-O0`, `-O1`, and `-O2` (`make checkall`)


---
# Limitations

Misses pointers where you do naughty things:
- pointers stored on disk and read back later
- pointers written to DMA-visible memory
- weird pointer encodings, tagging, xor tricks
- non-4-byte-aligned pointer conventions
- but not: multiple stacks / threads (since allocated :).

Solution: for today, don't do illegal things :)

---
# Final Summary

- `ckalloc` makes heap objects visible
- conservative mark-and-sweep approximates liveness in plain C
- leak detection and garbage collection differ mostly in the sweep action
- the main difficulty is not the graph walk
- the main difficulty is **accurately identifying roots and candidate pointers**

> Most GC failures in this lab are really accounting failures
