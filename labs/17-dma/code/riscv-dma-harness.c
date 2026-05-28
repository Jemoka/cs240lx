#include "rpi.h"
#include "cycle-count.h"
#include "memmap.h"

#include "printf.h"
void _putchar(char c) { uart_put8(c); }

#include "dma-impl.h"
#include "riscv-dma.h"
#include "riscv-dma-impl.h"
#include "ctx.h"

struct opaque_emulator {
  dma_ch_t *ch;
  cb_t *fst;
};

void riscv_emulate(
  struct emulator *emu,
  struct riscv_emulate_result *result
) {
  cycle_cnt_init();
  uint32_t start, end;
  start = cycle_cnt_read();
  dma_initiate_raw(emu->opaque->ch, emu->opaque->fst);
  while(!dma_done(emu->opaque->ch))
    ;
  end = cycle_cnt_read();
  result->nanos = end-start;
  dev_barrier();
}

enum {
  DMA_CH = 4
};

void riscv_emulator_init(
  struct emulator *emu
) {
  ctx_t ctx;
  ctx_init(&ctx, 64*1024);

  riscv_dma_init_all_tables();

  emu->opaque->ch = dma_init(DMA_CH);
  emu->opaque->fst = ctx_here(&ctx);
  riscv_generate_emulator(&ctx);
  uint32_t cb_cnt = ctx_here(&ctx) - emu->opaque->fst;
  printf("RISC-V: Total block count: %d\n", cb_cnt);
}

// clang-format off
enum { X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19,
  X20, X21, X22, X23, X24, X25, X26, X27, X28, X29, X30, X31 };
enum { ZERO = X0, RA = X1, SP = X2, GP = X3, TP = X4, T0 = X5, T1 = X6, T2 = X7, S0 = X8, FP = X8,
  S1 = X9, A0 = X10, A1 = X11, A2 = X12, A3 = X13, A4 = X14, A5 = X15, A6 = X16, A7 = X17, S2 = X18,
  S3 = X19, S4 = X20, S5 = X21, S6 = X22, S7 = X23, S8 = X24, S9 = X25, S10 = X26, S11 = X27,
  T3 = X28, T4 = X29, T5 = X30, T6 = X31 };
// clang-format on

struct pc_expect {
#define PC_EXPECT 1
#define PC_EXPECT_REL 2
  uint32_t flags;
  uint32_t value;
};
#define RV_PC_ANY ((struct pc_expect){0, 0})
#define RV_PC_NEXT ((struct pc_expect){PC_EXPECT | PC_EXPECT_REL, 4})
#define RV_PC_REL(off) ((struct pc_expect){PC_EXPECT | PC_EXPECT_REL, (off)})
#define RV_PC_ABS(off) ((struct pc_expect){PC_EXPECT, (off)})
static _Bool check_pc_expect(struct pc_expect expect, uint32_t pre, uint32_t post, uint32_t *expected) {
  if (expect.flags & PC_EXPECT) {
    if (expect.flags & PC_EXPECT_REL) {
      *expected = pre + expect.value;
      return post == pre + expect.value;
    } else {
      *expected = expect.value;
      return post == expect.value;
    }
  } else
    return 1;
}

struct mem_expect {
  volatile uint32_t *ptr;
  uint32_t expect;
};

static volatile uint32_t riscv_tests_run = 0, riscv_tests_passed = 0, riscv_tests_failed = 0;
static void riscv_test_summary() {
  printf("\n=== ADAPTED RISC-V ISA TEST SUMMARY ===\n");
  printf("\x1b[32m%d\x1b[0m/%d tests passed, \x1b[31m%d\x1b[0m/%d tests failed\n",
         riscv_tests_passed, riscv_tests_run, riscv_tests_failed, riscv_tests_run);
}

extern volatile struct rv_state RISCV[1];

void riscv_test_expect(struct emulator *emu, const char *test_name, uint32_t init[32],
                       uint32_t expect[32], uint32_t program[],
                       struct pc_expect expect_pc, struct mem_expect expect_mem[],
                       size_t expect_mem_cnt) {
  for (int i = 0; i < 32; i++)
    RISCV->REGS[i] = init[i];
  RISCV->PC = bus(program)._0;
  riscv_tests_run += 1;

  printf("\n=== TEST \x1b[35m%s\x1b[0m ===\n", test_name);
  uint32_t initial_pc = bus(program)._0;
  struct riscv_emulate_result result;
  riscv_emulate(
    emu,
    &result
  );
  uint32_t final_pc = RISCV->PC;
  printf("Finished in %d nanos\n", result.nanos);

  printf("Dumping final register state:\n");
  _Bool correct = 1, linebad = 0;
  for (int i = 0; i < 32; i++) {
    printf("%sx%d=", i < 10 ? " " : "", i);
    if (RISCV->REGS[i] == expect[i]) {
      printf("%08x\t", RISCV->REGS[i]);
    } else {
      printf("\x1b[31m%08x\x1b[0m\t", RISCV->REGS[i]);
      correct = 0;
      linebad = 1;
    }
    if ((i % 8) == 7) {
      printf("\n");
      if (linebad) {
        for (int j = i & (~7); j <= i; j++) {
          if (RISCV->REGS[j] != expect[j])
            printf("    \x1b[32m%08x\x1b[0m\t", expect[j]);
          else
            printf("            \t");
        }
        printf("\n");
      }
      linebad = 0;
    }
  }
  // if (expect_status != dbg_sta) {
  //   printf("DBG STATUS MISMATCH: got \x1b[31m%s\x1b[0m (expected %s)\n", dbg_sta_str(dbg_sta),
  //          dbg_sta_str(expect_status));
  //   correct = 0;
  // }
  uint32_t expected_pc_raw;
  if (!check_pc_expect(expect_pc, initial_pc, final_pc, &expected_pc_raw)) {
    printf("PC MISMATCH: got \x1b[31m%08x\x1b[0m (expected \x1b[32m%08x\x1b[0m)\n", final_pc,
           expected_pc_raw);
    printf("expect_pc=(%x) %x initial=%x final=%x\n", expect_pc.flags, expect_pc.value, initial_pc,
           final_pc);
    correct = 0;
  }
  for (int i = 0; i < expect_mem_cnt; i++) {
    if (*expect_mem[i].ptr != expect_mem[i].expect) {
      printf("MEM MISMATCH: at \x1b[35m%p\x1b[0m: \x1b[31m%08x\x1b[0m (expected "
             "\x1b[32m%08x\x1b[0m)\n",
             expect_mem[i].ptr, *expect_mem[i].ptr, expect_mem[i].expect);
      correct = 0;
    }
  }
  if (correct) {
    riscv_tests_passed += 1;
    printf("RESULT: \x1b[32mSUCCESS!!!\x1b[0m\n");
  } else {
    riscv_tests_failed += 1;
    printf("RESULT: \x1b[31mFAILURE!!!\x1b[0m\n");
  }
  return;
}

#define EMU_HALT_PASS 0x0000000b
#define EMU_HALT_FAIL 0x0000002b
#define R(...) (uint32_t[32]) { __VA_ARGS__ }
#define P(...) (uint32_t[]) { __VA_ARGS__, EMU_HALT_PASS }

#define REGTEST(name, inp, exp, prog) riscv_test_expect(emu, name, inp, exp, prog, RV_PC_NEXT, (struct mem_expect[]){0}, 0)
#define REGTEST_PC(name, inp, exp, prog, exppc) riscv_test_expect(emu, name, inp, exp, prog, exppc, (struct mem_expect[]){0}, 0)
#define MEMTEST_1(name, inp, exp, prog, memad, memexp) riscv_test_expect(emu, name, inp, exp, prog, RV_PC_NEXT, (struct mem_expect[]){{memad, memexp}}, 1)

static void run_rv32i_tests(struct emulator *emu);
static void run_rv32a_tests(struct emulator *emu);

// xxd -i hello_from_dma.bin
unsigned char hello_from_dma_bin[] = {
  0xb7, 0x53, 0x21, 0x7e, 0x93, 0x83, 0x03, 0x04, 0x97, 0x02, 0x00, 0x00,
  0x93, 0x82, 0x02, 0x02, 0x03, 0xc3, 0x02, 0x00, 0x63, 0x08, 0x03, 0x00,
  0x93, 0x82, 0x12, 0x00, 0x23, 0xa0, 0x63, 0x00, 0x6f, 0xf0, 0x1f, 0xff,
  0x0b, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x66, 0x72,
  0x6f, 0x6d, 0x20, 0x44, 0x4d, 0x41, 0x21, 0x0a, 0x00, 0x00, 0x01, 0x00
};
unsigned int hello_from_dma_bin_len = 60;

void notmain() {
  kmalloc_init_set_start(&__heap_start__, 128 * 1024 * 1024);

  struct opaque_emulator opaque_emu = {};
  struct emulator emu = { .opaque = &opaque_emu, };
  riscv_emulator_init(&emu);

  // for(int i=0;i<33;i++)
  //   RISCV->REGS[i] = 0x00;
  // RISCV->PC = bus(hello_from_dma_bin)._0;
  // RISCV->CYCLEL = 0;

  // struct riscv_emulate_result result;
  // riscv_emulate(
  //   &emu,
  //   &result
  // );

  // uart_flush_tx();

  // printf("cycle64: %d\n", RISCV->CYCLEL);
  // printf("Nanoseconds spent emulating: %d\n", result.nanos);

  run_rv32i_tests(&emu);
  run_rv32a_tests(&emu);

  riscv_test_summary();
}

static void run_rv32i_tests(struct emulator *emu) {
  printf("\n--- RV32I tests ---\n");

  REGTEST("rv32i_lui", R(), R([X3] = 0x12345000), P(0x123451b7)); /* lui x3,0x12345 */

  uint32_t auipc_program[] = {0x12345197}; /* auipc x3,0x12345 */
  REGTEST("rv32i_auipc", R(), R([X3] = bus(auipc_program)._0 + 0x12345000), auipc_program);

  uint32_t jal_program[] = {0x008001ef}; /* jal x3,.+8 */
  REGTEST_PC("rv32i_jal", R(), R([X3] = bus(jal_program)._0 + 4), jal_program, RV_PC_REL(8));
  uint32_t jal_back_program[] = {0xffdff1ef}; /* jal x3,.-4 */
  REGTEST_PC("rv32i_jal_backward", R(), R([X3] = bus(jal_back_program)._0 + 4), jal_back_program,
             RV_PC_REL((uint32_t)-4));

  uint32_t jalr_program[] = {0x00c081e7}; /* jalr x3,12(x1) */
  uint32_t jalr_pc = bus(jalr_program)._0;
  REGTEST_PC("rv32i_jalr_low_bit_clear", R([X1] = jalr_pc + 5),
             R([X1] = jalr_pc + 5, [X3] = jalr_pc + 4), jalr_program, RV_PC_ABS(jalr_pc + 16));

  REGTEST_PC("rv32i_beq_taken", R([X1] = 5, [X2] = 5), R([X1] = 5, [X2] = 5), P(0x00208463,EMU_HALT_FAIL,EMU_HALT_PASS),
             RV_PC_REL(8)); /* beq x1,x2,.+8 */
  REGTEST("rv32i_beq_not_taken", R([X1] = 5, [X2] = 6), R([X1] = 5, [X2] = 6), P(0x00208463,EMU_HALT_PASS,EMU_HALT_FAIL));
  uint32_t rv32i_beq_backward_taken[] = { EMU_HALT_PASS, 0xfe208ee3, EMU_HALT_FAIL, };
  REGTEST_PC("rv32i_beq_backward_taken", R([X1] = 9, [X2] = 9), R([X1] = 9, [X2] = 9),
             rv32i_beq_backward_taken+1, RV_PC_REL((uint32_t)-4)); /* beq x1,x2,.-4 */

  REGTEST_PC("rv32i_bne_taken", R([X1] = 5, [X2] = 6), R([X1] = 5, [X2] = 6), P(0x00209463,EMU_HALT_FAIL,EMU_HALT_PASS),
             RV_PC_REL(8)); /* bne x1,x2,.+8 */
  REGTEST("rv32i_bne_not_taken", R([X1] = 5, [X2] = 5), R([X1] = 5, [X2] = 5), P(0x00209463,EMU_HALT_PASS,EMU_HALT_FAIL));
  uint32_t rv32i_bne_backward_taken[] = { EMU_HALT_PASS, 0xfe209ee3, EMU_HALT_FAIL, };
  REGTEST_PC("rv32i_bne_backward_taken", R([X1] = 9, [X2] = 10), R([X1] = 9, [X2] = 10),
             rv32i_bne_backward_taken+1, RV_PC_REL((uint32_t)-4)); /* bne x1,x2,.-4 */

  REGTEST_PC("rv32i_blt_taken", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1),
             P(0x0020c463,EMU_HALT_FAIL,EMU_HALT_PASS), RV_PC_REL(8)); /* blt x1,x2,.+8 */
  REGTEST("rv32i_blt_not_taken", R([X1] = 1, [X2] = 0xffffffff), R([X1] = 1, [X2] = 0xffffffff),
          P(0x0020c463,EMU_HALT_PASS,EMU_HALT_FAIL));

  REGTEST_PC("rv32i_bge_taken", R([X1] = 1, [X2] = 0xffffffff), R([X1] = 1, [X2] = 0xffffffff),
             P(0x0020d463,EMU_HALT_FAIL,EMU_HALT_PASS), RV_PC_REL(8)); /* bge x1,x2,.+8 */
  REGTEST("rv32i_bge_not_taken", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1),
          P(0x0020d463,EMU_HALT_PASS,EMU_HALT_FAIL));

  REGTEST_PC("rv32i_bltu_taken", R([X1] = 1, [X2] = 0xffffffff), R([X1] = 1, [X2] = 0xffffffff),
             P(0x0020e463,EMU_HALT_FAIL,EMU_HALT_PASS), RV_PC_REL(8)); /* bltu x1,x2,.+8 */
  REGTEST("rv32i_bltu_not_taken", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1),
          P(0x0020e463,EMU_HALT_PASS,EMU_HALT_FAIL));

  REGTEST_PC("rv32i_bgeu_taken", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1),
             P(0x0020f463,EMU_HALT_FAIL,EMU_HALT_PASS), RV_PC_REL(8)); /* bgeu x1,x2,.+8 */
  REGTEST("rv32i_bgeu_not_taken", R([X1] = 1, [X2] = 0xffffffff), R([X1] = 1, [X2] = 0xffffffff),
          P(0x0020f463,EMU_HALT_PASS,EMU_HALT_FAIL));

  volatile uint32_t load_words[3] = {0x807f8180, 0x11223344};
  uint32_t load_base = bus(&load_words[0])._0;
  REGTEST("rv32i_lb_sign_extend", R([X1] = load_base), R([X1] = load_base, [X3] = 0xffffff81),
          P(0x00108183)); /* lb x3,1(x1) */
  REGTEST("rv32i_lbu_zero_extend", R([X1] = load_base), R([X1] = load_base, [X3] = 0x00000081),
          P(0x0010c183)); /* lbu x3,1(x1) */
  REGTEST("rv32i_lh_sign_extend", R([X1] = load_base), R([X1] = load_base, [X3] = 0xffff807f),
          P(0x00209183)); /* lh x3,2(x1) */
  REGTEST("rv32i_lhu_zero_extend", R([X1] = load_base), R([X1] = load_base, [X3] = 0x0000807f),
          P(0x0020d183)); /* lhu x3,2(x1) */
  REGTEST("rv32i_lw", R([X1] = load_base), R([X1] = load_base, [X3] = 0x11223344),
          P(0x0040a183)); /* lw x3,4(x1) */
  REGTEST("rv32i_lb_high_byte", R([X1] = load_base), R([X1] = load_base, [X3] = 0xffffff80),
          P(0x00308183)); /* lb x3,3(x1) */
  REGTEST("rv32i_lbu_low_byte", R([X1] = load_base), R([X1] = load_base, [X3] = 0x00000080),
          P(0x0000c183)); /* lbu x3,0(x1) */

  volatile uint32_t sb_mem = 0xaabbccdd;
  uint32_t sb_base = bus(&sb_mem)._0;
  MEMTEST_1("rv32i_sb", R([X1] = sb_base, [X2] = 0x00000055), R([X1] = sb_base, [X2] = 0x00000055),
            P(0x002080a3), &sb_mem, 0xaabb55dd); /* sb x2,1(x1) */

  volatile uint32_t sh_mem = 0xaabbccdd;
  uint32_t sh_base = bus(&sh_mem)._0;
  MEMTEST_1("rv32i_sh", R([X1] = sh_base, [X2] = 0x00001234), R([X1] = sh_base, [X2] = 0x00001234),
            P(0x00209123), &sh_mem, 0x1234ccdd); /* sh x2,2(x1) */

  volatile uint32_t sb_hi_mem = 0xaabbccdd;
  uint32_t sb_hi_base = bus(&sb_hi_mem)._0;
  MEMTEST_1("rv32i_sb_high_byte", R([X1] = sb_hi_base, [X2] = 0x00000055),
            R([X1] = sb_hi_base, [X2] = 0x00000055), P(0x002081a3), &sb_hi_mem,
            0x55bbccdd); /* sb x2,3(x1) */

  volatile uint32_t sh_low_mem = 0xaabbccdd;
  uint32_t sh_low_base = bus(&sh_low_mem)._0;
  MEMTEST_1("rv32i_sh_low_half", R([X1] = sh_low_base, [X2] = 0x00001234),
            R([X1] = sh_low_base, [X2] = 0x00001234), P(0x00209023), &sh_low_mem,
            0xaabb1234); /* sh x2,0(x1) */

  volatile uint32_t sw_mem[2] = {0xaabbccdd, 0x00000000};
  uint32_t sw_base = bus(&sw_mem[0])._0;
  MEMTEST_1("rv32i_sw", R([X1] = sw_base, [X2] = 0x89abcdef), R([X1] = sw_base, [X2] = 0x89abcdef),
            P(0x0020a223), &sw_mem[1], 0x89abcdef); /* sw x2,4(x1) */

  volatile uint32_t lw_neg_words[2] = {0xdecafbad, 0x00000000};
  uint32_t lw_neg_base = bus(&lw_neg_words[1])._0;
  REGTEST("rv32i_lw_negative_offset", R([X1] = lw_neg_base),
          R([X1] = lw_neg_base, [X3] = 0xdecafbad), P(0xffc0a183)); /* lw x3,-4(x1) */

  volatile uint32_t sw_neg_words[2] = {0x00000000, 0xaabbccdd};
  uint32_t sw_neg_base = bus(&sw_neg_words[1])._0;
  MEMTEST_1("rv32i_sw_negative_offset", R([X1] = sw_neg_base, [X2] = 0x13579bdf),
            R([X1] = sw_neg_base, [X2] = 0x13579bdf), P(0xfe20ae23), &sw_neg_words[0],
            0x13579bdf); /* sw x2,-4(x1) */

  REGTEST("rv32i_addi_negative", R([X1] = 0x00000020), R([X1] = 0x00000020, [X3] = 0x0000000f),
          P(0xfef08193)); /* addi x3,x1,-17 */
  REGTEST("rv32i_addi_max_imm", R([X1] = 1), R([X1] = 1, [X3] = 0x00000800),
          P(0x7ff08193)); /* addi x3,x1,2047 */
  REGTEST("rv32i_addi_min_imm", R([X1] = 0x00001000), R([X1] = 0x00001000, [X3] = 0x00000800),
          P(0x80008193)); /* addi x3,x1,-2048 */
  REGTEST("rv32i_slti_true", R([X1] = 0xfffffffe), R([X1] = 0xfffffffe, [X3] = 1),
          P(0xfff0a193)); /* slti x3,x1,-1 */
  REGTEST("rv32i_slti_false", R([X1] = 0), R([X1] = 0, [X3] = 0), P(0xfff0a193));
  REGTEST("rv32i_sltiu_true", R([X1] = 0), R([X1] = 0, [X3] = 1),
          P(0x0010b193)); /* sltiu x3,x1,1 */
  REGTEST("rv32i_sltiu_false", R([X1] = 2), R([X1] = 2, [X3] = 0), P(0x0010b193));
  REGTEST("rv32i_sltiu_minus_one_true", R([X1] = 0xfffffffe), R([X1] = 0xfffffffe, [X3] = 1),
          P(0xfff0b193)); /* sltiu x3,x1,-1 */
  REGTEST("rv32i_sltiu_minus_one_false", R([X1] = 0xffffffff), R([X1] = 0xffffffff, [X3] = 0),
          P(0xfff0b193));
  REGTEST("rv32i_xori", R([X1] = 0x000000f0), R([X1] = 0x000000f0, [X3] = 0x000000a5),
          P(0x0550c193)); /* xori x3,x1,0x55 */
  REGTEST("rv32i_xori_minus_one", R([X1] = 0x12345678), R([X1] = 0x12345678, [X3] = 0xedcba987),
          P(0xfff0c193)); /* xori x3,x1,-1 */
  REGTEST("rv32i_ori", R([X1] = 0x000000a0), R([X1] = 0x000000a0, [X3] = 0x000000f5),
          P(0x0550e193)); /* ori x3,x1,0x55 */
  REGTEST("rv32i_ori_sign_imm", R([X1] = 0x00000700), R([X1] = 0x00000700, [X3] = 0xffffff00),
          P(0x8000e193)); /* ori x3,x1,-2048 */
  REGTEST("rv32i_andi", R([X1] = 0x000000f0), R([X1] = 0x000000f0, [X3] = 0x00000050),
          P(0x0550f193)); /* andi x3,x1,0x55 */
  REGTEST("rv32i_andi_sign_imm", R([X1] = 0x1234567f), R([X1] = 0x1234567f, [X3] = 0x12345670),
          P(0xff00f193)); /* andi x3,x1,-16 */
  REGTEST("rv32i_slli", R([X1] = 0xff), R([X1] = 0xff, [X3] = 0x00007f80),
          P(0x00709193)); /* slli x3,x1,7 */
  REGTEST("rv32i_srli", R([X1] = 0xff000000), R([X1] = 0xff000000, [X3] = 0x01fe0000),
          P(0x0070d193)); /* srli x3,x1,7 */
  REGTEST("rv32i_srai", R([X1] = 0xff000000), R([X1] = 0xff000000, [X3] = 0xfffe0000),
          P(0x4070d193)); /* srai x3,x1,7 */

  REGTEST("rv32i_add", R([X1] = 1, [X2] = 2), R([X1] = 1, [X2] = 2, [X3] = 3),
          P(0x002081b3)); /* add x3,x1,x2 */
  REGTEST("rv32i_sub", R([X1] = 1, [X2] = 2), R([X1] = 1, [X2] = 2, [X3] = 0xffffffff),
          P(0x402081b3)); /* sub x3,x1,x2 */
  REGTEST("rv32i_sll", R([X1] = 1, [X2] = 31), R([X1] = 1, [X2] = 31, [X3] = 0x80000000),
          P(0x002091b3)); /* sll x3,x1,x2 */
  REGTEST("rv32i_sll_mask_zero", R([X1] = 1, [X2] = 32), R([X1] = 1, [X2] = 32, [X3] = 1),
          P(0x002091b3)); /* sll x3,x1,x2 */
  REGTEST("rv32i_slt", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1, [X3] = 1),
          P(0x0020a1b3)); /* slt x3,x1,x2 */
  REGTEST("rv32i_sltu", R([X1] = 0xffffffff, [X2] = 1), R([X1] = 0xffffffff, [X2] = 1, [X3] = 0),
          P(0x0020b1b3)); /* sltu x3,x1,x2 */
  REGTEST("rv32i_xor", R([X1] = 0x00ff00ff, [X2] = 0x0f0f0f0f),
          R([X1] = 0x00ff00ff, [X2] = 0x0f0f0f0f, [X3] = 0x0ff00ff0),
          P(0x0020c1b3)); /* xor x3,x1,x2 */
  REGTEST("rv32i_srl", R([X1] = 0x80000000, [X2] = 31), R([X1] = 0x80000000, [X2] = 31, [X3] = 1),
          P(0x0020d1b3)); /* srl x3,x1,x2 */
  REGTEST("rv32i_srl_mask_31", R([X1] = 0x80000000, [X2] = 0x000000ff),
          R([X1] = 0x80000000, [X2] = 0x000000ff, [X3] = 1), P(0x0020d1b3)); /* srl x3,x1,x2 */
  REGTEST("rv32i_sra", R([X1] = 0x80000000, [X2] = 31),
          R([X1] = 0x80000000, [X2] = 31, [X3] = 0xffffffff), P(0x4020d1b3)); /* sra x3,x1,x2 */
  REGTEST("rv32i_sra_mask_zero", R([X1] = 0x87654321, [X2] = 0x00000040),
          R([X1] = 0x87654321, [X2] = 0x00000040, [X3] = 0x87654321),
          P(0x4020d1b3)); /* sra x3,x1,x2 */
  REGTEST("rv32i_or", R([X1] = 0xf000000f, [X2] = 0x0ff00ff0),
          R([X1] = 0xf000000f, [X2] = 0x0ff00ff0, [X3] = 0xfff00fff),
          P(0x0020e1b3)); /* or x3,x1,x2 */
  REGTEST("rv32i_and", R([X1] = 0xff00ff00, [X2] = 0x0f0f0f0f),
          R([X1] = 0xff00ff00, [X2] = 0x0f0f0f0f, [X3] = 0x0f000f00),
          P(0x0020f1b3)); /* and x3,x1,x2 */

  REGTEST("rv32i_x0_hardwired", R([X1] = 1, [X2] = 2), R([X1] = 1, [X2] = 2),
          P(0x00208033)); /* add x0,x1,x2 */
  REGTEST("rv32i_x0_source", R([X1] = 0xfeedface), R([X1] = 0xfeedface, [X3] = 0xfeedface),
          P(0x000081b3)); /* add x3,x1,x0 */

  // Not sure how tf to test this; for now, we just have a stub
  REGTEST("rv32i_fence", R(), R(), P(0x0ff0000f));      /* fence iorw,iorw */
  REGTEST("zifencei_fence_i", R(), R(), P(0x0000100f)); /* fence.i */
}

static void run_rv32a_tests(struct emulator *emu) {
  printf("\n--- RV32A tests ---\n");

  volatile uint32_t sc_fail_mem = 0x01020304;
  uint32_t sc_fail_addr = bus(&sc_fail_mem)._0;
  MEMTEST_1("rv32a_sc_w_without_reservation",
              R([X1]=sc_fail_addr,[X2]=0xaabbccdd),
              R([X1]=sc_fail_addr,[X2]=0xaabbccdd,[X3]=1),
              P(0x1820a1af),
              &sc_fail_mem,
              0x01020304); /* sc.w x3,x2,(x1): failure code is any nonzero value. */

  volatile uint32_t lrsc_mem = 0x11111111;
  uint32_t lrsc_addr = bus(&lrsc_mem)._0;
  printf("\n--- RV32A LR/SC ordered reservation sequence ---\n");
  REGTEST("rv32a_lr_w",
          R([X1]=lrsc_addr),
          R([X1]=lrsc_addr,[X3]=0x11111111),
          P(0x1000a1af)); /* lr.w x3,(x1) */
  MEMTEST_1("rv32a_sc_w_after_lr",
          R([X1]=lrsc_addr,[X2]=0x22222222),
          R([X1]=lrsc_addr,[X2]=0x22222222,[X3]=0),
          P(0x1820a1af),
          &lrsc_mem,
          0x22222222);
  // TODO: unclear what's necessary to clear a reservation
  MEMTEST_1("rv32a_sc_w_with_wrong_reservation",
              R([X1]=lrsc_addr+4,[X2]=0x33333333),
              R([X1]=lrsc_addr+4,[X2]=0x33333333,[X3]=1),
              P(0x1820a1af),
              &lrsc_mem,
              0x22222222);

  volatile uint32_t aqrl_mem = 0x00000010;
  uint32_t aqrl_addr = bus(&aqrl_mem)._0;
  printf("\n--- RV32A aq/rl ordered reservation sequence ---\n");
  REGTEST("rv32a_lr_w_aq",
          R([X1]=aqrl_addr),
          R([X1]=aqrl_addr,[X3]=0x00000010),
          P(0x1400a1af)); /* lr.w.aq x3,(x1) */
  MEMTEST_1("rv32a_sc_w_rl",
          R([X1]=aqrl_addr,[X2]=0x00000020),
          R([X1]=aqrl_addr,[X2]=0x00000020,[X3]=0),
          P(0x1a20a1af),
          &aqrl_mem,
          0x00000020); /* sc.w.rl x3,x2,(x1) */

  volatile uint32_t amoswap_mem = 0x10203040;
  uint32_t amoswap_addr = bus(&amoswap_mem)._0;
  MEMTEST_1("rv32a_amoswap_w",
          R([X1]=amoswap_addr,[X2]=0x50607080),
          R([X1]=amoswap_addr,[X2]=0x50607080,[X3]=0x10203040),
          P(0x0820a1af),
          &amoswap_mem,
          0x50607080); /* amoswap.w x3,x2,(x1) */

  volatile uint32_t amoadd_mem = 5;
  uint32_t amoadd_addr = bus(&amoadd_mem)._0;
  MEMTEST_1("rv32a_amoadd_w",
          R([X1]=amoadd_addr,[X2]=7),
          R([X1]=amoadd_addr,[X2]=7,[X3]=5),
          P(0x0020a1af),
          &amoadd_mem,
          12); /* amoadd.w x3,x2,(x1) */

  volatile uint32_t amoadd_wrap_mem = 0xffffffff;
  uint32_t amoadd_wrap_addr = bus(&amoadd_wrap_mem)._0;
  MEMTEST_1("rv32a_amoadd_w_wrap",
          R([X1]=amoadd_wrap_addr,[X2]=2),
          R([X1]=amoadd_wrap_addr,[X2]=2,[X3]=0xffffffff),
          P(0x0020a1af),
          &amoadd_wrap_mem,
          1); /* amoadd.w x3,x2,(x1) */

  volatile uint32_t amoxor_mem = 0xff00ff00;
  uint32_t amoxor_addr = bus(&amoxor_mem)._0;
  MEMTEST_1("rv32a_amoxor_w",
          R([X1]=amoxor_addr,[X2]=0x0f0f0f0f),
          R([X1]=amoxor_addr,[X2]=0x0f0f0f0f,[X3]=0xff00ff00),
          P(0x2020a1af),
          &amoxor_mem,
          0xf00ff00f); /* amoxor.w x3,x2,(x1) */

  volatile uint32_t amoand_mem = 0xff00ff00;
  uint32_t amoand_addr = bus(&amoand_mem)._0;
  MEMTEST_1("rv32a_amoand_w",
          R([X1]=amoand_addr,[X2]=0x0f0f0f0f),
          R([X1]=amoand_addr,[X2]=0x0f0f0f0f,[X3]=0xff00ff00),
          P(0x6020a1af),
          &amoand_mem,
          0x0f000f00); /* amoand.w x3,x2,(x1) */

  volatile uint32_t amoor_mem = 0xf000000f;
  uint32_t amoor_addr = bus(&amoor_mem)._0;
  MEMTEST_1("rv32a_amoor_w",
          R([X1]=amoor_addr,[X2]=0x0ff00ff0),
          R([X1]=amoor_addr,[X2]=0x0ff00ff0,[X3]=0xf000000f),
          P(0x4020a1af),
          &amoor_mem,
          0xfff00fff); /* amoor.w x3,x2,(x1) */

  volatile uint32_t amomin_mem = 5;
  uint32_t amomin_addr = bus(&amomin_mem)._0;
  MEMTEST_1("rv32a_amomin_w",
          R([X1]=amomin_addr,[X2]=0xfffffffe),
          R([X1]=amomin_addr,[X2]=0xfffffffe,[X3]=5),
          P(0x8020a1af),
          &amomin_mem,
          0xfffffffe); /* amomin.w x3,x2,(x1) */

  volatile uint32_t amomin_old_min_mem = 0x80000000;
  uint32_t amomin_old_min_addr = bus(&amomin_old_min_mem)._0;
  MEMTEST_1("rv32a_amomin_w_signed_old_min",
          R([X1]=amomin_old_min_addr,[X2]=0x7fffffff),
          R([X1]=amomin_old_min_addr,[X2]=0x7fffffff,[X3]=0x80000000),
          P(0x8020a1af),
          &amomin_old_min_mem,
          0x80000000); /* amomin.w x3,x2,(x1) */

  volatile uint32_t amomax_mem = 0xfffffffe;
  uint32_t amomax_addr = bus(&amomax_mem)._0;
  MEMTEST_1("rv32a_amomax_w",
          R([X1]=amomax_addr,[X2]=5),
          R([X1]=amomax_addr,[X2]=5,[X3]=0xfffffffe),
          P(0xa020a1af),
          &amomax_mem,
          5); /* amomax.w x3,x2,(x1) */

  volatile uint32_t amominu_mem = 0xffffffff;
  uint32_t amominu_addr = bus(&amominu_mem)._0;
  MEMTEST_1("rv32a_amominu_w",
          R([X1]=amominu_addr,[X2]=2),
          R([X1]=amominu_addr,[X2]=2,[X3]=0xffffffff),
          P(0xc020a1af),
          &amominu_mem,
          2); /* amominu.w x3,x2,(x1) */

  volatile uint32_t amomaxu_mem = 1;
  uint32_t amomaxu_addr = bus(&amomaxu_mem)._0;
  MEMTEST_1("rv32a_amomaxu_w",
          R([X1]=amomaxu_addr,[X2]=0x80000000),
          R([X1]=amomaxu_addr,[X2]=0x80000000,[X3]=1),
          P(0xe020a1af),
          &amomaxu_mem,
          0x80000000); /* amomaxu.w x3,x2,(x1) */

  volatile uint32_t amomaxu_old_max_mem = 0xffffffff;
  uint32_t amomaxu_old_max_addr = bus(&amomaxu_old_max_mem)._0;
  MEMTEST_1("rv32a_amomaxu_w_old_max",
          R([X1]=amomaxu_old_max_addr,[X2]=0),
          R([X1]=amomaxu_old_max_addr,[X2]=0,[X3]=0xffffffff),
          P(0xe020a1af),
          &amomaxu_old_max_mem,
          0xffffffff); /* amomaxu.w x3,x2,(x1) */

  volatile uint32_t amoadd_aqrl_mem = 0x00000010;
  uint32_t amoadd_aqrl_addr = bus(&amoadd_aqrl_mem)._0;
  MEMTEST_1("rv32a_amoadd_w_aqrl",
          R([X1]=amoadd_aqrl_addr,[X2]=0x00000020),
          R([X1]=amoadd_aqrl_addr,[X2]=0x00000020,[X3]=0x00000010),
          P(0x0620a1af),
          &amoadd_aqrl_mem,
          0x00000030); /* amoadd.w.aqrl x3,x2,(x1) */
}
