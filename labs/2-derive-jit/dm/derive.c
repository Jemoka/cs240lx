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

/// mov imm rot4 derivations ///
#define MOV_DEST(n) "mov r"#n", r0, ROR #1;\n"
DERIVE(mov_dest, MOV_DEST, 16)

#define MOV_SRC(n) "mov r0, r" #n ", ROR #1;\n"
DERIVE(mov_imm, MOV_SRC, 16)

#define MOV_ROT(n) "mov r0, r1, ROR #"#n";\n"
DERIVE(mov_rot, MOV_ROT, 31)

uint32_t gen_mov_rot(uint32_t dst, uint32_t src, uint32_t rot) {
    derived mov_dest = derive_mov_dest();
    derived mov_imm = derive_mov_imm();
    derived mov_rot = derive_mov_rot();

    uint32_t template = mov_dest.template; 
    template &= mov_dest.unchanged;
    template &= mov_imm.unchanged;
    template &= mov_rot.unchanged;

    template = setmask(template, mov_dest.changed, dst);
    template = setmask(template, mov_imm.changed, src);
    template = setmask(template, mov_rot.changed, rot);

    return template;    
}

// test instruction
GETOPCODE(mov_imm_rot, "mov r3, r4, ROR #8")

void notmain() {
    demand(get_add_579() == gen_add(5, 7, 9), "ADD derivations borked?");
    demand(get_mov_imm_rot() == gen_mov_rot(3, 4, 8), "MOV derivations borked?");
};  

