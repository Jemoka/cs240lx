## Using gcc to instrument every function call.

For any file compiled with the `finstrument-functions` flag:
```
gcc -finstrument-functions -c file.c
```

The `gcc` compiler will insert every routine with two calls:
```c
// Called immediately after entering a function.
void __cyg_profile_func_enter(void *this_fn, void *call_site); 
// — Called just before exiting a function. ￼
void __cyg_profile_func_exit(void *this_fn, void *call_site); 
```

You can look at `test-fn.c` for how to do this to print call graphs.

### Footgun: instrumenting the instrumentation code

You can do this ability to do a all sorts of tricks.  Though, also, to
shoot lots of stuff by accident.  For example if you look at the `.list`
file you see that in fact `gcc` also instruments these instrumentation
routines, causing a nice infinite loop:

```
00008038 <__cyg_profile_func_exit>:
    8038:   e92d4070    push    {r4, r5, r6, lr}
    803c:   e1a0400e    mov r4, lr
    8040:   e59f5018    ldr r5, [pc, #24]   ; 8060 <__cyg_profile_func_exit+0x28>
    8044:   e1a0100e    mov r1, lr
    8048:   e1a00005    mov r0, r5
    804c:   eb000004    bl  8064 <__cyg_profile_func_enter>
    8050:   e1a01004    mov r1, r4
    8054:   e1a00005    mov r0, r5
    8058:   ebfffff6    bl  8038 <__cyg_profile_func_exit>
    805c:   e8bd8070    pop {r4, r5, r6, pc}
    8060:   00008038    .word   0x00008038

00008064 <__cyg_profile_func_enter>:
    8064:   e92d4070    push    {r4, r5, r6, lr}
    8068:   e1a0400e    mov r4, lr
    806c:   e59f5018    ldr r5, [pc, #24]   ; 808c <__cyg_profile_func_enter+0x28>
    8070:   e1a0100e    mov r1, lr
    8074:   e1a00005    mov r0, r5
    8078:   ebfffff9    bl  8064 <__cyg_profile_func_enter>
    807c:   e1a01004    mov r1, r4
    8080:   e1a00005    mov r0, r5
    8084:   ebffffeb    bl  8038 <__cyg_profile_func_exit>
    8088:   e8bd8070    pop {r4, r5, r6, pc}
    808c:   00008064    .word   0x00008064
```

But, while gcc taketh, it also giveth and we can tell it to not do
this obviously bad thing:

```c
// gcc will happily instrument its own instrumentation code, smh.
void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_fn, void *call_site);
void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_fn, void *call_site);
```

### Footgun: calling instrumentated code

Once we are instrumented code: if we call other routines,
they will also infinite loop as above, so we have to turn them
off as well statically (which can be a hassle), or dynamically
using a flag.  (This should remind you about the early 
140e lab where we did linker tricks for tracing.)

