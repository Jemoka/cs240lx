#ifndef __eraser_h__
#define __eraser_h__

enum {
    SH_INVALID     = 1 << 0,
    SH_VIRGIN     = 1 << 1,      // writeable, allocated
    SH_FREED       = 1 << 2,
    SH_SHARED = 1 << 3,
    SH_EXCLUSIVE   = 1 << 4,
    SH_SHARED_MOD      = 1 << 5,
};

typedef struct {
    unsigned char state;
    unsigned char tid;
    unsigned short ls;
} eraser_state_t;

void eraser_init(void);
void eraser_onalloc(void *addr);
void eraser_onfree(void *addr);
void eraser_set_thread_id(char id);
void lock(void *lockobj);
void unlock(void *lockobj);
char eraser_get_state(void *addr);

#endif 
