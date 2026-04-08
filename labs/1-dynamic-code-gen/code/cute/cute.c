#include "rpi.h"
#include "derive.h"

/// add derivations ///
#define ADD_DST(n) "add r"#n", r0, r0;\n"
DERIVE(add_dst, ADD_DST, 16)

#define ADD_SRC1(n) "add r0, r"#n", r0;\n"
DERIVE(add_src1, ADD_SRC1, 16)

#define ADD_SRC2(n) "add r0, r0, r"#n";\n"
DERIVE(add_src2, ADD_SRC2, 16)

uint32_t gen_add(uint32_t dst, uint32_t src1, uint32_t src2) {
    derived add_dst = derive_add_dst();
    derived add_src1 = derive_add_src1();
    derived add_src2 = derive_add_src2();

    uint32_t template = add_dst.template; 
    template &= add_dst.unchanged;
    template &= add_src1.unchanged;
    template &= add_src2.unchanged;

    template = setmask(template, add_dst.changed, dst);
    template = setmask(template, add_src1.changed, src1);
    template = setmask(template, add_src2.changed, src2);

    return template;    
}

// test instruction
GETOPCODE(add_579, "add r5, r7, r9")

////////////////////////////////////////////////////////////////

/// mov imm ///
#define MOVIMM_DST(n) "mov r"#n", #1;\n"
DERIVE(movimm_dst, MOVIMM_DST, 16)

#define MOVIMM_SRC(n) "mov r0, #" #n ";\n"
DERIVE(movimm_src, MOVIMM_SRC, 16)

uint32_t gen_mov_imm(uint32_t dst, uint32_t imm) {
    derived movimm_dst = derive_movimm_dst();
    derived movimm_src = derive_movimm_src();

    uint32_t template = movimm_dst.template; 
    template &= movimm_dst.unchanged;
    template &= movimm_src.unchanged;

    template = setmask(template, movimm_dst.changed, dst);
    template = setmask(template, movimm_src.changed, imm);

    return template;    
}

/// mov imm ///
#define MOV_DST(n) "mov r"#n", r0;\n"
DERIVE(mov_dest, MOV_DST, 16)

#define MOV_SRC(n) "mov r0, r" #n ";\n"
DERIVE(mov_src, MOV_SRC, 16)

uint32_t gen_mov(uint32_t dst, uint32_t src) {
    derived mov_dest = derive_mov_dest();
    derived mov_src = derive_mov_src();

    uint32_t template = mov_dest.template; 
    template &= mov_dest.unchanged;
    template &= mov_src.unchanged;

    template = setmask(template, mov_dest.changed, dst);
    template = setmask(template, mov_src.changed, src);

    return template;    
}


////////////////////////////////////////////////////////////////

// ldr a2, [a3, #3]
#define LDR_DST(n) "ldr r" #n ", [r0, #1];\n"
DERIVE(ldr_dst, LDR_DST, 16)

#define LDR_SRC(n) "ldr r0, [r" #n ", #1];\n"
DERIVE(ldr_src, LDR_SRC, 16)

#define LDR_OFF(n) "ldr r0, [r0, #" #n "];\n"
DERIVE(ldr_off, LDR_OFF, 16)

uint32_t gen_ldr(uint32_t dst, uint32_t src, uint32_t off) {
    derived ldr_dst = derive_ldr_dst();
    derived ldr_src = derive_ldr_src();
    derived ldr_off = derive_ldr_off();

    uint32_t template = ldr_dst.template; 
    template &= ldr_dst.unchanged;
    template &= ldr_src.unchanged;
    template &= ldr_off.unchanged;

    template = setmask(template, ldr_dst.changed, dst);
    template = setmask(template, ldr_src.changed, src);
    template = setmask(template, ldr_off.changed, off);

    return template;    
}

GETOPCODE(ldr_a2_a3_4, "ldr r2, [r3, #4]")


////////////////////////////////////////////////////////////////

// str a2, [a3, #3]
#define STR_DST(n) "str r" #n ", [r0, #1];\n"
DERIVE(str_dst, STR_DST, 16)

#define STR_SRC(n) "str r0, [r" #n ", #1];\n"
DERIVE(str_src, STR_SRC, 16)

#define STR_OFF(n) "str r0, [r0, #" #n "];\n"
DERIVE(str_off, STR_OFF, 16)

uint32_t gen_str(uint32_t dst, uint32_t src, uint32_t off) {
    derived str_dst = derive_str_dst();
    derived str_src = derive_str_src();
    derived str_off = derive_str_off();

    uint32_t template = str_dst.template; 
    template &= str_dst.unchanged;
    template &= str_src.unchanged;
    template &= str_off.unchanged;

    template = setmask(template, str_dst.changed, dst);
    template = setmask(template, str_src.changed, src);
    template = setmask(template, str_off.changed, off);

    return template;    
}

GETOPCODE(str_a2_a3_4, "str r2, [r3, #4]")

////////////////////////////////////////////////////////////////

GETOPCODE(bxlr, "bx lr")


typedef uint32_t (*code_fn)(uint32_t*);
code_fn setup(uint32_t* codebuf) {
    uint32_t *code = codebuf;
    // load 4 into reg3
    *codebuf++ = gen_mov_imm(2, 4);
    // load codebuf into r1
    *codebuf++ = gen_ldr(1, 0, 0);
    // add 1 to r1 and store back in codebuf
    *codebuf++ = gen_add(1, 1, 2);
    *codebuf++ = gen_str(1, 0, 0);
    // move r2 into r0 (the return value)
    *codebuf++ = gen_mov(0, 2);
    // return
    *codebuf++ = get_bxlr();
    return (void *) code;
}

uint32_t lmao(uint32_t* codebuf) {
    code_fn fn = (code_fn) codebuf;
    return fn(codebuf);
}

void notmain() {
    /* demand(gen_ldr(2, 3, 4) == get_ldr_a2_a3_4(), "LDR derivations borked?"); */
    /* demand(gen_str(2, 3, 4) == get_str_a2_a3_4(), "STR derivations borked?"); */
    uint32_t codebuf[128] = {0};
    code_fn fn = setup(codebuf); 

    printk("Hi: %u\n", lmao(codebuf));
    printk("hi: %u\n", lmao(codebuf));
    printk("hi: %u\n", lmao(codebuf));
    printk("hi: %u\n", lmao(codebuf));
    printk("hi: %u\n", lmao(codebuf));
};  

