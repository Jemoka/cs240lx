## Errata

***NOTE:***
  - The watchpoint example was adapted from single stepping,
    and unfortunately has a couple stupid stale comments about breakpoints
    and single-stepping.  We are doing watchpoints, not single stepping.
    We will be getting watchpoint exceptions (a subset of data abort
    faults), not breakpoint exceptions (which are a subset of prefetch
    abort faults).

## Memory tracing

Today you'll use the ARMv6 domain faults and debug watchpoints to make
your own simple memory tracing system that traps every load and store.
This will let you build a variety of kernel level checking tools easily in
a couple hundred lines of code.  In comparison, doing similar tools with
dynamic binary rewriting (such as Valgrind or Pin) will take thousands
of lines for the checker, close to a million lines for the base system,
and generally can't run on kernel code.

You'll use this memory trapping system to build a simple little memory
tracing system (`memtrace.c`) that can run different client checkers on
memory operations.

Next lab we'll use it to write a simple checker (`checker-purify.c`)
that runs on this system and flags whens a load or store references
outside of blocks of memory allocated using a slightly modified version
of your checking allocator.

***Checkoff***:
  - Write your own tracing system based on the examples.
  - Write some tests that shows your system works.
  - Port it to the `memtrace` system we defined (or write your own
    interface!).
  - Ideally: Do some kind of extension.

I'll add more extensions, but here's a few:
  - Add transparent logging for device memory.  Tag the device
    memory with its own domain, and turn it off and on.
  - Change the code to look at the instruction and figure out the
    complete set of bytes that it reads and writes. E.g., `push` can
    push many registers (and access many words), `pop` can pop many
  - If you get motivated, it's an interesting puzzle to do it the
    old way: use single stepping to turn off traps, jump to the memory
    instruction in single step mode, then come back and turn traps
    back on.
  - Make your own `watchpoint.c` using the debug lab code
    you wrote in 140e.
  - Ambitious extension: use single stepping to verifiy that the code
    computes exactly the same results when run with trapping and without.

    How: hash all registers on each single step fault, and combine it with
    the previous hash (equivalance hashing from 140e), For deterministic
    code, you should get the same hash with and without trapping.

    You can look at `example-single-step` for how to do this (original
    prelab).

------------------------------------------------------------------------
#### Background: the big idea for memory tracing.

If you can see every load or store you can check many things: race
conditions, memory corruption, use of tainted values, etc.

Unfortunately, the traditional way tools such as valgrind and pin trace
these is very complicated.  They use dynamic translations (for code
discovery) and then some form of instruction emulation to determine an
instruction's effects.  It's hard to do the first in few lines of code
and figuring out ARM memory operations is a significant pain.

We'll use domain trapping to trace all memory operations to a
contiguous region of memory and then single stepping to have the
CPU do the operation for us:
   1. To trace a region: give it a unique domain and then remove
      permissions for that domain.  Everything will run as before, until
      there is a load or store to the range, upon which we get a fault.

      Recall: ARM has 16 domains Each page is tagged with a domain.
      You can switch domain permissions with a single CP15 register write
      (fast).

   2. We then call a client handler on the trapped address; the client
      code can then do whatever checking or modification it wnats.

   3. After the code returns: we need to emulate the trapped instruction.
      We can't just jump back or we'd trap again.  On a simple
      architecture (e.g., riscv) we could just emulate the instruction.
      However, we have ARM.  Fortunately we can use watchpoint faults.
      The interesting thing about watchpoints: if you set a watchpoint
      on address X, you get the watchpoint fault *after the memory
      operation runs*.  This means you can use them to run an instruction
      you don't understand, but immediately get control back after it 
      executes.  

      For our case: set a watchpoint fault on the address, disable domain
      trapping, jump back, instruction, get a data abort (watchpoint)
      fault, and then reenable domain trapping and resume until the next
      load or store.

#### Reiterate: trapping each load and store.

We just said this, but to try to make it crystaline clear we'll re-iterate.

Idea 1: Fast trapping of heap accesses:
  1. Give the heap its own domain id.   
  2. To trap heap access: remove permissions for that domain.
  3. To allow heap access: enable permissions for that domain.
  4. Result: changing heap permission is fast: a single domain register write.

Idea 2: How to trace all heap memory operations:
  1. Turn off access for the heap domain.
  2. Start running the code.
  3. If the code doesn't load or store to the heap (does ALU operations,
     or memory operations to the stack, global memory, GPIO etc) will
     get no traps and run at full speed (very different from valgrind!).
  4. If the code does load or store the heap, you will get a domain section trap.

Idea 3: How to execute a trapping memory instruction:
  1. In the data abort fault handler: Enable permissions for the heap
     domain so the code can read or write the heap.
  2. Before you jump back: Set a watchpoint fault on the address that caused
     the domain fault.
  3. Jump back to the faulting pc to run the memory instruction.
     The hardware will execute that one single instruction and then
     immediately throw a watchpoint fault (handled in data abort).
     give a single-step mismatch exception.
  4. In the resulting data abort handler, now remove access to the heap
     domain id and disable the watchpoint breakpoint.  You can now jump
     back to the pc given for the watchpoint fault and the the code will
     continue until the next heap access.

------------------------------------------------------------------------
### Part 0: watchpoint and domain crash course.

Our goal will be: trap every heap load or store so we can check if they
are to legal, allocated blocks.  The lab uses a bunch of machinery from
140e, but there are two main tricks: the first is how to trap every access
(we use domains), the second is how to execute it (we use watchpoints).

To understand watchpoints: 
 - Go through the code in `example-watchpt` to understand what it is
   doing, make some changes, see that you get expected results.

To understand memory trapping and domain faults:
  - Go through the code in `example-trap` to understand what it is doing,
    make some changes, see that you get expected results.  You'll be
    stealing most of this code for Part 1.

As a good first step before part 1: seperate the trap/watchpoint code
from the code being used to illustrate it and write a couple test cases.

------------------------------------------------------------------------
### Part 1: combine watchpoint and trapping into a simple interface

Note:
  - There is no checked-in directory for this part!  You'll have to copy
    makefiles and define your own header file, tests etc.

Before building the whole system, we'll first take the examples above
and combine them into a simple mem-trace interface that will trap on
all heap addresses as discussed in the backgrond section above.
  1. Combine both examples into a system that will trace heap memory faults.
  2. It should implement the following initialization routine (called
     by the client test case):
```
        void memtrace_init(void);
```
     That will do a one-time initialization of everything (VM, heap,
     exception handlers) that is needed.  You can just steal all the
     code for this from the examples.  In a real system these pieces
     would be sharded-out in a better way, but for now we cut corners.
  3. On memory traps (before running watchpoint) it should call the client
     routine (this will be in each test case):
```
            void memtrace_handler(regs_t *r, uint32_t fault_addr, int load_p);
```
     With the fault registers `r`, the faulting address `fault_addr`
     and whether the fault was a load or store.
  4. Implement a:
```
    // trapping on
    void memtrace_trap_enable(void);
    // trapping off
    void memtrace_trap_disable(void);
```
  5. Rewrite the example's code plus a couple other examples to show it
     show that your implementation works.


I would make a copy of the trap example and add the pieces of the
watchpoint example that you need --- I think about 50 lines of code
in total.

Rewriting it in this way will hopefully give an easy, active way to
understand the concepts (better than reading manauls!).

------------------------------------------------------------------------
### Part 2.  write `code/memtrace.c`

***NOTE: I'm going to add an example here.  Do a pull if see this.***

Now, we'll extend your previous code into a simple memory tracing system
that makes it easy to drop in new checkers.  The interface is in
`code/memtrace.h`. 

The initialization routine:
```
    void memtrace_init( void *data,
        memtrace_fn_t pre,
        memtrace_fn_t post,
        unsigned trap_dom);
```
Takes:
  - `data`: a pointer to data that gets passed to client handlers
    `pre` and `post`.
  - `pre`: called when a memory instruction traps, before running the
     instruction.
  - `post`: called when a memory instruction traps, after running the
     instruction.
  - `trap_dom`: the domain id associated with all trapping memory.

At least one of `pre` and `post` should be defined.  For today, this 
routine calls the code to initialize the virtual memory system.
The handlers are called with a `memtrace.h:fault_ctx` structure
that takes provides:
  - `r`: a pointer to the current fault regs.
  - `pc`: the initial fault pc (note the fault regs pc value `r->regs[15]`
    will differ in post since that gets called after the instruction.
  - `addr`: the memory address of the fault.
  - `nbytes`: the size of the access.  NOTE: for right now we always pass 4, 
     but this is not correct.   A good extension is to make it not so stupid.
  - `load_p`: whether it was a load (`load_p=1`) or store (`load_p=0`).

Like your original system is provides routines for 
  1. Turning trapping on: `memtrace_trap_enable`.
  2. And off:`memtrace_trap_disable`.

Big picture:
  - For today, `memtrace_init` sets up virtual memory by calling
    `sbrk-trap.c:sbrk_init`.    This isn't how we'd to it for real since
    it makes things hard to compose, but keeps today more simple.

  - `sbrk_init` calls the same VM code as we used last lab
    (`vm_map_everything`).  `vm_map_everything` allocates a 1MB heap
    with its own private domain id (so we can easily trap accesses to it).

    `sbrk_init` also allocates a second 1MB used by its trivial 
    non-trapping heap allocator (`notrap_alloc`).  

    The only interesting thing about these two heaps is that there is
    a 1MB unmapped zone between them so overflows from the regular heap
    don't get into our non-trapping heap easily.

  - Extension: If you want to use shadow memory, the easiest thing is to
    map another 1MB (using `memmap-default.c:mb_map`) so you can add a
    constant offset to heap addresses to get their associated shadow.

    Alternatively you could cap the main heap size and devote an
    equivalant amount of memory from the non-trapping heap to it.

To understand the interface, the easiest thing is to look at the couple
of tests in `tests-memtrace`.

What is success:
  1. The few tests in `tests-memtrace` pass. 

------------------------------------------------------------------------
#### Extension: compute the actual number of bytes accessed.

The biggest limit of the current code is that it doesn't correctly
compute how many bytes an instruction accesses.  For this you'll parse
the machine instruction and determine how many bytes it accesses.
You should write some test code that shows that you do this correctly.

(If you are blocked on this I do have a header for it.)

-------------------------------------------------------------------------------
#### Extension: replace a bunch of our `.o` files.

We use a bunch of code from old labs.  You should already have versions,
so can start dropping in yours instead of ours.

------------------------------------------------------------------------
#### Extension: simple device memory checker.

Device memory should only be written with a str instruction and only
loaded with a ldr instrution --- no `push`, `pop`, load byte store byte
etc.  This is a pretty  simple checker that gets you used to messing
around with domains.

  1. Put the device memory in its own domain.
  2. Pass this domain ID to the memtrace checker.
  3. Check every device access to make sure it uses the right
     instructions.
  4. Run a bunch of device code and flag errors. 
  5. To catch compiler bugs, you can redefine `put32`, `get32` etc
     in the `rpi.h` header to be inline functions that take volatile
     32-bit pointers and perform the assignment (put32) or dereference
     (get32).  This will give many more opportunities for the compiler
     to cause problems.
  6. Even more fancy: you can extend the checker to detect when we read
     or write one device A and then read or write a second device
     B without performing a memory barrier.  You'd just need to 
     add code that detects when you call the memory barrier code.
     (You could override the routine, rewrite it using your JIT 
     knowledge, or possibly add single-stepping to look for the 
     barrier instruction.)

You will probably want to modify `memmap-default.c` so that it tags
BCM memory better.  You may need to put these in domain 0 since we map
device memory (BCM) using 16MB memory supersections and the arm1176 doc
states supersections have a domain 0.  (Note, I haven't tested if this
is true for pinned mappings or only true for page tables.)
------------------------------------------------------------------------
#### Extension: fancier device memory checker

A very common, nasty problem in embedded is that the code uses pointers
to manipulate device memory, but either the programmer does not use
`volatile` correctly or the compiler has a bug.   

Device memory bugs are very nasty and also very easy to make since you
are doing stuff to memory that the compiler thinks is redundant.  For
example:
  - Multiple back-to-back writes to the same location, so the compiler
    believes it just needs to do the last write.  Examples: 
    both the UART and i2c fifo queue.
  - Multiple reads to the same location without an intervening
    write (so that the compiler believes it can remove them).  This
    came up when we did mailbox and UART "is there space" checks.

As a recent example, several people had device i2c bugs because they
were sloppy with not using `volatile`.

Using your memory tracing you can write a checker for this pretty
easily.  The key property we will exploit is that `volatile` acceses
are invariant across optimization levels.  The compiler cannot remove,
reorder, or add them.  

How:
  1. Modify the memtrace code so that it can do memory trapping on
     device memory.  This shouldn't require much work, but you will
     have to be careful for circularity problems.
  2. Run your device driver and log the device addresses read
     and written along with the values.
  3. As a sanity check: Re-run the device code against this log and
     for each read, return the value read, and for each write, check that
     the written value matches the log.  If the code is deterministic
     and your code doesn't have bugs, this replay should succeed no
     matter how many times you do it.
  4. Now recompile the code with different optimization levels ("-O0",
     "-O1", "-Ofast" etc) and rerun it against the log.  (You'll have to
     ship the log over with your binary.) If any read or write changes
     you know there is a bug.

------------------------------------------------------------------------
#### Extensions: protect system memory.

Right now, the client code --- which is presumably buggy or we wouldn't
be running a bug finder on it --- can corrupt our checker memory.
We can extend our uses of domains to protect checker memory similarly
to how we trap on heap:
  1. Tag all checker memory with its own private domain ID.
  2. By default, disable access to this domain.
  3. When running the checker code, re-enable access to this domain.
     When done running the checker code, disable it.
  4. Any wild read or writes done by the client code will cause a fault.

You can extend this to protecting one peer subsystem from another.
(E.g., the virtual memory system from the file system and vice versa).

There are different lengths you can push this, up to the extreme of
protecting the stack, code and data and then running the untrusted code
at a lower privilege so that it can't arbitrarily give itself access.
Starts to look like a OS with user code :).  There are some linker script
tricks you can play to keep different subsystems isolated from each other.
Interesting exercise if you push it far.

------------------------------------------------------------------------
#### Extensions.

Adding more extensions.  If you see this sentence do a pull.
