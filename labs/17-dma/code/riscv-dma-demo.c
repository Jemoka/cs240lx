#include "rpi.h"
#include "cycle-count.h"
#include "memmap.h"

#include "printf.h"
#include "stamps.h"
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

  riscv_init_emulator_tables();
  staff_init_all_stamp_tables();

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

extern volatile struct rv_state RISCV[1];


#define EMU_HALT_PASS 0x0000000b
#define EMU_HALT_FAIL 0x0000002b

static void run_rv32i_tests(struct emulator *emu);
static void run_rv32a_tests(struct emulator *emu);

// xxd -i hello_from_dma.bin
unsigned char hello_from_dma2_bin[] = {
  0x17, 0x09, 0x00, 0x00, 0x13, 0x09, 0x89, 0x04, 0x83, 0x42, 0x09, 0x00,
  0x63, 0x94, 0x02, 0x00, 0x6f, 0x10, 0xc0, 0x04, 0x13, 0x09, 0x19, 0x00,
  0xef, 0x00, 0xc0, 0x00, 0x6f, 0xf0, 0xdf, 0xfe, 0x0b, 0x00, 0x00, 0x00,
  0x37, 0x54, 0x21, 0x7e, 0x13, 0x04, 0x04, 0x04, 0xb7, 0x54, 0x21, 0x7e,
  0x93, 0x84, 0x44, 0x06, 0x83, 0xae, 0x04, 0x00, 0x93, 0xfe, 0x2e, 0x00,
  0xe3, 0x8c, 0x0e, 0xfe, 0x23, 0x20, 0x54, 0x00, 0x67, 0x80, 0x00, 0x00,
  0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x44,
  0x4d, 0x41, 0x21, 0x0d, 0x0a, 0x00, 0x01, 0x00
};
unsigned int hello_from_dma2_bin_len = 92;

void notmain() {
  kmalloc_init_set_start(&__heap_start__, 128 * 1024 * 1024);

  struct opaque_emulator opaque_emu = {};
  struct emulator emu = { .opaque = &opaque_emu, };
  riscv_emulator_init(&emu);

  for(int i=0;i<33;i++)
    RISCV->REGS[i] = 0x00;
  RISCV->PC = bus(hello_from_dma2_bin)._0;
  RISCV->CYCLEL = 0;

  struct riscv_emulate_result result;
  riscv_emulate(
    &emu,
    &result
  );
  
  printf("\n\n--- DMA RISC-V Emulator Report ---\n");
  printf("Instructions retired: %d\n", RISCV->CYCLEL);
  printf("Nanoseconds spent emulating: %d ns\n", result.nanos);
  enum { NS_PER_S = 1000000000ULL };
  printf("Instruction throughput: %lld Hz\n",
    (uint64_t)RISCV->CYCLEL * (uint64_t)NS_PER_S / (uint64_t)result.nanos
  );
}
