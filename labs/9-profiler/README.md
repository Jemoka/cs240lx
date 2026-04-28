## Instruction level profiler

When I was a kid, we had a useful instruction-level profiler "pixie" on
SGI workstations that would do binary rewriting and count the exact number
of times you ran each individual machine code instruction in your code.
It was exact in that it counted everything, rather than a statistical
profiler like `gprof` that only sampled.

Pixie was great.  There was nothing I couldn't speed up by many factors
if I used it.  There is some chance that without it I wouldn't have
gotten into MIT for grad school (and thus here to this lab at Stanford)
since my demonstrated competence was making stuff fast and without pixie
this would have been hard.  So I have a fondness for it.

Of course, SGI died and binaries became hard to rewrite so you probably
haven't used anything like it.  Also, such things generally don't run
on kernel code.

So today we'll build a simple version.  Using the ARM single-stepping
hardware makes it pretty easy.  If you add the counters from the ARM
performance monitor, things get interesting in that it's easy to see why
different parts of the code are taking longer than others.  And it's not
too hard to make it run on kernel code --- in fact, unusually, it's in
some ways easier to trace kernel code versus user-level code since there are
fewer moving parts and we have complete control.

Various external motivations:
  - You'll need to understand exception handling when we build 
    our memory tracing tools in the next few labs.  
  - It's been a minute (or a lifetime for some) since you've seen
    ARM1176 exception handling, so we review it with a worked out 
    example before jumping into the fancier hacks.
  - A profiler is a nice way to kick the hardware around and see how
    things work.

#### Background

Code:
  - `code`: The initial code in `code/` gives a working single step
    example for today's lab.  You'll be extending it.
  - `0-crash-course`: a complete working example for how to use single
    stepping from 140E 2026.  Good to refresh, or to kick the tires more.
  - For both: you're encouraged to change them, add prints, remove them,
    trace the execution.

Docs:
  - The arm1176 performance monitor unit: [pmu][pmu].  This gives
    many different counters --- data cache misses, branch mispredictions,
    etc.  Useful for extending the profiler.

Useful 140e labs --- worth reading after the lab for next time,
even if you don't need them this time:
  - [interrupts][interrupts]: the interrupt/exception lab provides a 
    working interrupt example and discusses shadow registers.
  - [single-step][single-step]: this covers the single step hardware
    (with an example) and covers how to build the breakpoint veneer.
  - [debug-hw][debug-hw]: this covers the debug hardware more generally.

#### Checkoff

  1. Write a simple instruction profiler similar to the `gprof` in 140e
  2. Add the cycle counter and display both the number of times 
     each instruction runs and the cycle counts.
  3. Add another interesting hardware counter to (2).
  4. Correct both (2) and (3) as much as you can so the numbers get
     close to the true values.  If you push this correction far
     that counts as the extension below:
  5. Do some extension.  

There are tons of extensions.  Will add!  Or ask.

------------------------------------------------------------------
### Background 1: Single step example: `code/`

We give a complete working single-step example in the `code/`
directory. It's called "ss-pixie" (single-step pixie) in honor of its
ancestor.

The code has two public routines:
  - `pixie_start()`: start single step tracing
  - `unsigned pixie_stop()`: stop tracing and return the number of instructions
    executed.

At a high level it works by using "mismatch faults" to single-step
through code between these two calls.

As you might recall from 140e (labs 9, 10, 11), mismatch faults only
trigger for code running at user level, so at a high level the code
works as follows:
  1. `pixie_start`: initialization:
       1. Sets up the exception vectors to catch both mismatch
          faults and system calls (see below);
       2. Turns on the debug hardware and mismatch faulting;
       3. Finally switches to user mode.  (i.e., sets the mode field in
          `cpsr` register mode to `0b10000`)
  2. As soon as the code starts running at user level it will:
       1. Get a mismatch fault;
       2. Which will get vectored to the "prefetch abort" handler.  
  3. The `prefetch_abort`  handler:
       1. Counts the instructions;
       2. Sets a mismatch on the faulting instruction (so that the 
          faulting instruction will execute normally, but any other 
          instruction will fault).
  4. When the code being profiled wants to turn off profiling it:
       1. Calls `pixie_stop` to turn off mismatching and switch back to
          privileged mode;
       2. Since neither change can be done directly by unprivileged
          user code, `pixie_stop` elevates privilege in a standard way
          by invoking a system call.
       3. The system call turns off single stepping, and returns back
          to the calling code.  (NOTE: we could of course leave single
          stepping on as long as we run at privileged mode.  We could
          also not even use the system call and instead flip back
          to unprivileged execution when we see the `pc` hit the
          known `pixie_stop` address.)

You should poke around the example.  It's heavily commented.

------------------------------------------------------------------
### Background 2: 140e Single-step example: `0-crash-course/`

Seeing the same thing in two slightly different ways can help depth
perception.  In that spirit, we include a second single-step example
used as a crash-course for 140E 2026 lab 10.

You can view the code as an interactive textbook that shows how to
use single stepping. 

Go through the two programs in `0-crash-course`:
  - Start with `0-nop1-example.c` and see how it works,
    then `1-many-fn-example.c`
  - They are a couple hundred lines of code, but it's mostly
    comments.  You should add prints, change things, corrupt
    registers to see that the code actually uses them --- anything
    that makes it less passive.
  - In order to make your results easily comparable to other
    people the initial routines are written in assembly (so the compiler
    won't mess with them), and put in `single-step-start.S` so they will
    be at the same address for everyone (`single-step-start.o` is linked
    at the start of the program at the same address for everyone).
  - With that said: there is no limitation in the code --- you can
    single-step any code you can run at user level, whether it was written
    in C, rust, zig, or JIT'ed at runtime.

------------------------------------------------------------------
### Part 1: turn `ss-pixie` into an instruction profiler

Modify the `ss-pixie` code to count how many times each instruction
gets run.
  1. You can get the faulting PC address from the 15th register.
  2. Similar to the `gprof` code you wrote in 140e, you'll add an
     array that is the size of the code segment and increment it each
     time an instruction runs.

The basic algorithm:
 1. Use `kmalloc` (or, better: your ckalloc!) to allocate an array at
    least as big as the code.  (If you use kmalloc make sure to do
    `kmalloc_init` first.)  Compute the code size using the labels defined
    in `libpi/memmap` (we give C definitions in `libpi/include/memmap.h`).

 2. In the fault handler, use the program counter value (register 15)
    to index into this array and increment the associated count.  

    NOTE: it's very easy to mess up sizes.  Each instruction is 4 bytes, so
    you'll divide the `pc` by 4.  You'll want to subtract where the code
    starts (from our `memmap` we expect `__code_start__` to be `0x8000`).

 3. `pixie_dump(N)`: the easiest acceptable way to build this is to 
    just dump out any instruction with a higher than N count in order.
    We will take this.

    A more client friendly implementation is to print out the top,
    sorted, non-zero values in this array along with the `pc` value they
    correspond to.

    In any case: You should be able to look in the disassembled code
    (the `.list` file for each routine) to see which instruction these
    correspond to.

 4. For the simple test `tests/1-prof-test.c` that just repeatedly prints,
    the expected result is that most counts should be in `PUT32`, `GET32`,
    and various `uart` routines.  Note: you will get different 
    results when you have caching enabled or not (why?).

Write a couple tests and validate that your profiler eats them and spits
out interesting values.  For interesting tests, please post to Ed so we
can steal them (add your name / year).

--------------------------------------------------------------------
### A great extension: Make your profiler legible.

Have a wrapper around `my-install` that passes each PC address to the GNU
utility `arm-none-eabi-addr2line` to convert the addresses to file and
line number information.  You can even open the given files and display
the code on one side, and the counts on the other side.  (Super useful!)

For example, when I run `1-prof-test.c` with caching enabled I get:

        ...a bunch of values...

	    pc=0x96d4: cnt=164
	    pc=0x96d8: cnt=164
	    pc=0x96dc: cnt=164
	    pc=0x96e0: cnt=164
	    pc=0x96e4: cnt=164
	    pc=0x96e8: cnt=164
	    pc=0x96ec: cnt=164

        ...another bunch of values...


When I use "addr2line" to get the file and function of the first address
`0x96d4` by running:

        % arm-none-eabi-addr2line 0x96d4 -s -f  \
                  -e objs/l1/l2/l3/tests/1-prof-test.elf

I get:
```
        uart_can_putc
        uart.c:159
```

Which makes sense --- mostly the code is waiting to be able to emit 
output.

If you want very useful --- you can precompute this information, concatenate
it to the end of the binary, and pull it into the code so it can self
report.  

Major extension:
  - Even more useful, but more challenging: use the debug info in
    the ELF binary.  (You'll have to change your bootloader to handle ELF
    files).


------------------------------------------------------------------
### Part 2: add support for cycle counters

Counting instructions is good, but we would also like to count the number
of cycles each instruction costs.  When there is a big difference between 
these it's interesting.

We have cycle counter routines in `libpi/include/cycle-count.h` so it
might seem easy.  Unfortunately, our single step handler does so much 
compared to running a single instruction that just recording cycles in 
the handler would be useless.

Ideally, what we would want to do instead is:
  1. At the first line of the fault handler, record the cycle count.
  2. At the last line of the fault handler, record the cycle count.
  3. Subtracting (1) from (2) gives the cycles it costs to return
     from the exception, run the next instruction, and then get
     another fault.  Now, while it still includes the overhead
     of taking and returning from an exception, these costs are large
     but low variance (more below).
  4. Fun challenge: try to correct (3) so that it gives you close
     to the true cycle count values.  You will have to write some
     known code (e.g., with just nops) and measure it with cycle
     counters and work out the rough constant correction.  Note:
     since our monitor code is fairly heavy-weight (e.g., we take
     an exception for each instruction) this won't be exact.
     (NB: Unless you are exceptionally clever.)

How can we do this?  Various problems:
  1. How can we read the cycle counter when we get an exception?
     All the registers are live!  Fortunately, while we can't use `lr`,
     we have a private `sp` and since the ARM registers are untyped we
     can read into it.  (Weird, but legal.)  
  2. Ok, we have the cycle counter in `sp`: how do we store it?  We
     need a stack to push it onto, but the stack needs `sp`.

     Fortunately, the arm1176 provides (at least) three coprocessor
     scratch registers for "process and thread id's."  However, since
     the values are not interpreted by the hardware, they can be used
     to store arbitrary values.  The screenshot of page 3-129 (chapter
     3 of the arm 1176.pdf manual) below gives the instructions.

     (Note: this kind of ARM lore is a good reason to read chapter 3 of
     the arm1176: there are all sorts of weirdo little operations that
     when you add cleverness can let you do neat stuff not possible on
     a general purpose OS.)

<p align="center">
  <img src="images/global-regs.png" width="600" />
</p>


  3. Ok: so what about at the end?  We want the cycle counter read
     as close as possible to when we jump back.  If we put this reading
     in sp, we won't have any place to do the read in (1).   As you
     probably guessed we can put it in one of the other scratch registers.
     (Or maybe do something more clever?)

     Thus, the difference between (2) and (3) gives the number of cycles 
     from
       - A: when we return from a mismatch exception;
       - B: running the next instruction;
       - C: taking another mismatch exception and got back to 
         the handler.

     As mentioned above, while A and C are large compared to B, they are
     performed internally within the hardware itself and (appear to!) have
     low variance --- and low variance means they just add (roughly)
     a constant overhead, easily removed or ignored.  B on the other
     hand can vary significantly, which is what we are interested in.

     NOTE: if you find a case where A and C have significant variance,
     it is interesting --- let us know! 

#### What code to write

What to do:
  1. Use the scratch registers to record the cycle counter at the 
     start and end of the handler.  Try to write the code so 
     the reads are as close to the start and end as possible. 
     The cleaner you can do this, the more stable the measurements
     will be.
  2. Write some simple code that you know the answer to and validate
     that you get useful answers.
  3. After you're happy with (2), you can subtract off a correction
     factor.  Or, alternatively, just keep things as is and use the
     relative differences.

Some common bugs:
  1. If you notice an unusually large cycle value at a PC soon after
     the user switch: this is because the scratch register used to
     record the "last" cycle read has not been initialized.  Easy fix:
     before the `cps` instruction in `pixie_switchto_user_asm` read the
     cycle counter and set it.
  2. If you add a line comment `@` or `//` to a macro used in the
     `ss-pixie-asm.S` file, this will eat the entire remainder of
     the macro body!  So either don't do this, don't use a macro, or
     use `/* ... */` style comments.  If you're getting reset faults,
     after changing the trampoline macro, this is what is going on :).
  3. Note that for most approaches, the cycle counter differences
     will be for the *previous* instruction not the current one.
  4. Bugs in your profiler often won't lead to crashes, just
     wrong results.  And wrong results are hard to spot if you don't know
     what the right one is.   So write some very very simple tests where
     you can see what is going on and validate that what you expect is
     what you get.  For example, `4-nop-test.c` profiles a routine that
     does 10 nops, no loads or stores with caching enabled.  We expect
     each `nop` instruction to take about the same (for me 36 cycles).
     Any big spike is a sign that something is off.

------------------------------------------------------------------
## Part 3: use your profiler to speed up some code.

Required:
  1. Make custom versions of the GPIO set and clear routines as
     inline routines in a `gpio-raw.h`.    They should do no 
     error checking and just do raw volatile reads and writes of
     the needed addresses.  Use your profiler to measure their cost.
     We will need these in the next lab.  Keep an eye out for loads of
     large GPIO address constants --- a common ARM way to slow things
     down.
  2. NOTE: these are simple enough the profiler isn't a huge win here
     so maybe not the best example :) --- feel free to do one of the
     ones below.

Suggestions:
  1. Make a very fast memcpy, possibly tuned to 8 byte copies.
  2. Make a faster malloc that wraps up `kr-malloc` and uses an
     array indexed by different sizes (e.g., 8, 16, 24, 32, ... 512?)
     with a fast linked list for each bucket.  This is a common
     optimization. Useful for this class!
  3. Something else interesting!

------------------------------------------------------------------
### Extension: Implement PMU counters `code-pmu`

The arm1176 has a bunch of interesting performance counters
we can use to see what is going on, such as cache misses, TLB misses,
prediction misses, procedure calls.  You'll write a simple library that
exposes these, which will make it much easier to optimize code.

The arm1176 document describes the performance monitor unit (PMU) on
pages 3-133 --- 3-140 (see `./docs` in this lab or the `arm1176.pdf`
manual in the class `./docs`).  It has many useful performance counters,
though with the limit that only two can be enabled at any time in addition
to the "always on" cycle counter.  We'll write a simple interface to
expose these.

These counters make it much easier to speed up your code --- it's hard to
know the right optimization when you don't know what the bottleneck is.
In addition, they can be used to test your understanding of the hardware
--- if you believe you understand how the BTB, TLB, or cache works, write
code based on this understanding and measure if the expected result is
the actual.

Use them to track some interesting counter (e.g., cache miss, branch
misprediction) and use that to trace where your code does bad stuff.
It's best to also use the cycle counter so you can get a detailed view.

To debug:
  1. First write some simple code and measure it without the
     profiler to make sure you have code that triggers the counter.
  2. Then rerun with the profiler and check it shows what you are
     measuring.
  3. Then do some other interesting code.

What you need:
   1. Performance monitor control register (3-133): write this to
      select which performance counters to use (the values are in table
      3-137) and to enable the PMU at all.
   2. Cycle counter register (3-137): read this to get the current
      cycle count.
   3. Count register 0 (3-138): read this to get the 32-bit 0-event
      counter (set in step 1).
   4. Count register 1 (3-139): read this to get the 32-bit 1-event
      counter (set in step 1).

This is a choose-your-own adventure:  look through the counters and
write some code that shows off something they can measure (or run on your
140e or 240lx code!).  The easiest way is to copy the header files into
`libpi/include` directory and they should just work.

------------------------------------------------------------------
### Extension: Do a hierarchical profiler

Seeing that a given instruction is run a lot is great, but if it's in
a routine called by many other routines you can't easily figure out how
to optimize.   A very useful tool is a hierarchical profiler that tracks
who called what, and when they did, how much it cost.  You can do a full
graph, or do 2 deep, 3 deep, etc.  Any of them will be extremely useful.

A great use of this is to apply it to your fat32 file system and use it
to speed it up.  Massive improvements are possible!

------------------------------------------------------------------
### Hard Extension: make the profiler able to profile itself

This is an interesting challenge.  It's possible, but you need some
clever recursive thinking.  Sai (one of our God-level TAs) did this for
an LX side-quest.

Assume profiler-A is profiling profiler-B.  You'll have to do some
virtualization tricks (similar to a full VMM) so that profiler-A can
emulate / virtualize the privileged instructions and exceptions that
profiler-B uses:  
  - breakpoint instructions.
  - reads/writes of cpsr, spsr.
  - the cps instruction.
  - others?
  - You'll also have to use different memory (stack, etc)

Very interesting if you can do more than two!

This is a hard extension.  Interesting final project.

[interrupts]: https://github.com/dddrrreee/cs140e-26win/tree/main/labs/4-interrupts
[single-step]: https://github.com/dddrrreee/cs140e-26win/tree/main/labs/10-interleave-checker
[debug-hw]: https://github.com/dddrrreee/cs140e-26win/tree/main/labs/11-debug-hw
[pmu]: ./docs/pmu-ch3-arm1176.pdf

