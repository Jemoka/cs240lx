#include "rpi.h"

// gcc will happily instrument its own instrumentation code, smh.
void __attribute__((no_instrument_function)) 
__cyg_profile_func_enter(void *this_fn, void *call_site);
void __attribute__((no_instrument_function)) 
__cyg_profile_func_exit(void *this_fn, void *call_site);


static volatile int depth = 0, can_print_p = 0;;

// also don't instrument this helper.
__attribute__((no_instrument_function)) 
static inline void indent(int n) {
    while(n-- > 0)
        output(" ");
}

#define emit(args...) do {                  \
    indent(depth*5);                        \
    output("---> ");                        \
    debug(args);                            \
} while(0)

void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    // haven't started tracing yet.
    if(!can_print_p)
        return;
    depth++;

    can_print_p = 0;
    indent(depth*5); output("entering %x\n", this_fn);
    can_print_p = 1;
}
 
// — Called just before exiting a function. ￼
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    // haven't started tracing yet.
    if(!can_print_p)
        return;
    assert(depth);

    can_print_p = 0;
    indent(depth*5); output("leaving %x\n", this_fn);
    can_print_p = 1;
    depth--;

}

void a(void);
void b(void);
void c(void);

void notmain(void) {
    output("--------------------------------------\n");
    can_print_p = 1;
    emit("calling a:\n");
    a();
    can_print_p = 0;
    output("--------------------------------------\n");
}

void a(void) {
    emit("calling b:\n");
    b();
}
void b(void) {
    emit("calling c: \n");
    c();
}
void c(void) {
    emit("<done!>\n");
}
