#pragma once

#include "dma-impl.h"
#include "ctx.h"

void staff_stamp_inc8(ctx_t *ctx, bus_t rac, bus_t inp);
void staff_stamp_inc8_nocry(ctx_t *ctx, bus_t rac, bus_t inp);
void staff_stamp_inc(ctx_t *ctx, bus_t ret, bus_t inp);
void staff_stamp_inc64(ctx_t *ctx, bus_t ret, bus_t inp);
void staff_stamp_sub8(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs);
void staff_stamp_sub8_nocry(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs);
void staff_stamp_sub(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_cmp_bbb(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs);
void staff_stamp_geu_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags);
void staff_stamp_eq_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags);
void staff_stamp_ge_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags);
void staff_stamp_add8(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs);
void staff_stamp_add8_nocry(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs);
void staff_stamp_add(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_add64(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_not8(ctx_t *ctx, bus_t ret, bus_t arg);
void staff_stamp_not(ctx_t *ctx, bus_t ret, bus_t arg);
void staff_stamp_and8(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_and(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_or8(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_or(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_xor(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs);
void staff_stamp_sll8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_sll(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_srl8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_srl(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_sra8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_sra(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift);
void staff_stamp_neg(ctx_t *ctx, bus_t ret, bus_t arg);

void staff_init_all_stamp_tables();

static inline void stamp_inc8(ctx_t *ctx, bus_t rac, bus_t inp) { staff_stamp_inc8(ctx, rac, inp); }
static inline void stamp_inc8_nocry(ctx_t *ctx, bus_t rac, bus_t inp) { staff_stamp_inc8_nocry(ctx, rac, inp); }
static inline void stamp_inc(ctx_t *ctx, bus_t ret, bus_t inp) { staff_stamp_inc(ctx, ret, inp); }
static inline void stamp_inc64(ctx_t *ctx, bus_t ret, bus_t inp) { staff_stamp_inc64(ctx, ret, inp); }
static inline void stamp_sub8(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs) { staff_stamp_sub8(ctx, rac, lhs, rhs); }
static inline void stamp_sub8_nocry(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs) { staff_stamp_sub8_nocry(ctx, rac, lhs, rhs); }
static inline void stamp_sub(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_sub(ctx, ret, lhs, rhs); }


enum {
  CMP_X4 = 1 << 0,
  CMP_INV = 1 << 1,
};
static inline void stamp_geu_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags) { staff_stamp_geu_bww(ctx, ret, lhs, rhs, flags); }
static inline void stamp_eq_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags) { staff_stamp_eq_bww(ctx, ret, lhs, rhs, flags); }
static inline void stamp_ge_bww(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs, int flags) { staff_stamp_ge_bww(ctx, ret, lhs, rhs, flags); }

static inline void stamp_add8(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs) { staff_stamp_add8(ctx, rac, lhs, rhs); }
static inline void stamp_add8_nocry(ctx_t *ctx, bus_t rac, bus_t lhs, bus_t rhs) { staff_stamp_add8_nocry(ctx, rac, lhs, rhs); }
static inline void stamp_add(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_add(ctx, ret, lhs, rhs); }
static inline void stamp_add64(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_add64(ctx, ret, lhs, rhs); }
static inline void stamp_not8(ctx_t *ctx, bus_t ret, bus_t arg) { staff_stamp_not8(ctx, ret, arg); }
static inline void stamp_not(ctx_t *ctx, bus_t ret, bus_t arg) { staff_stamp_not(ctx, ret, arg); }
static inline void stamp_and8(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_and8(ctx, ret, lhs, rhs); }
static inline void stamp_and(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_and(ctx, ret, lhs, rhs); }
static inline void stamp_or8(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_or8(ctx, ret, lhs, rhs); }
static inline void stamp_or(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_or(ctx, ret, lhs, rhs); }
static inline void stamp_xor(ctx_t *ctx, bus_t ret, bus_t lhs, bus_t rhs) { staff_stamp_xor(ctx, ret, lhs, rhs); }
static inline void stamp_sll8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_sll8(ctx, ret, in, shift); }
static inline void stamp_sll(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_sll(ctx, ret, in, shift); }
static inline void stamp_srl8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_srl8(ctx, ret, in, shift); }
static inline void stamp_srl(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_srl(ctx, ret, in, shift); }
static inline void stamp_sra8(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_sra8(ctx, ret, in, shift); }
static inline void stamp_sra(ctx_t *ctx, bus_t ret, bus_t in, bus_t shift) { staff_stamp_sra(ctx, ret, in, shift); }
static inline void stamp_neg(ctx_t *ctx, bus_t ret, bus_t arg) { staff_stamp_neg(ctx, ret, arg); }
