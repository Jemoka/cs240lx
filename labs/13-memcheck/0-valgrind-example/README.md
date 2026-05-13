To appreciate how simple trapping makes things it's useful to glance
at complicated a dynamic binary rewriting tool is --- this directory
contains the "hello world" Valgrind `lackey` tool Valgrind provides as
the simplest tool you can write.  It doesn't check anything, but at 900
lines it (should be) larger than your entire system for today's lab ---
tracing, purify, etc.  And that is before you throw in the enormous
complexity of the Valgrind system itself (over 1M+ lines of extremely
tricky low-level code).
