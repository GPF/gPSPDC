#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define COMMON_H
#define CPU_H
#define function_cc

#ifdef _MSC_VER
/* Encoding tests do not call the declared noreturn dispatch routines. */
#define __attribute__(attributes)
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#endif
#include "../dc/sh4_emit.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

translation_ptr_t translation_ptr;

static u32 last_invalidate_addr;
static u32 last_invalidate_size;
static u32 invalidate_count;

void sh4_invalidate_icache_region(u32 addr, u32 size)
{
  last_invalidate_addr = addr;
  last_invalidate_size = size;
  invalidate_count++;
}

static u32 fatal_error_count;
static const char *fatal_error_detail;

void gpsp_dynarec_fatal_error(const char *detail)
{
  fatal_error_count++;
  fatal_error_detail = detail;
}

static u16 code_buffer[64];
static int failures;

static void reset_buffer(void)
{
  memset(code_buffer, 0, sizeof(code_buffer));
  translation_ptr = code_buffer;
}

static void expect_words(const char *name, const u16 *expected, size_t count)
{
  size_t emitted = (size_t)(translation_ptr - code_buffer);

  if(emitted != count || memcmp(code_buffer, expected, count * sizeof(expected[0])) != 0)
  {
    size_t i;

    printf("%s failed: emitted %zu words, expected %zu\n", name, emitted, count);
    for(i = 0; i < emitted || i < count; i++)
    {
      u16 got = (i < emitted) ? code_buffer[i] : 0xFFFF;
      u16 want = (i < count) ? expected[i] : 0xFFFF;
      printf("  [%zu] got %04x expected %04x\n", i, got, want);
    }

    failures++;
  }
  else
  {
    printf("%s: ok\n", name);
  }
}

static void test_load_store_encodings(void)
{
  const u16 expected[] = { 0x54C4, 0x1C55 };

  reset_buffer();
  SH4_EMIT_LW(sh4_reg_r4, sh4_reg_r12, 16);
  SH4_EMIT_SW(sh4_reg_r5, sh4_reg_r12, 20);
  expect_words("load/store displacement encodings", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_alu_encodings(void)
{
  const u16 expected[] = {
    0x345C, /* add r5,r4 */
    0x32CC, /* add r12,r2; preserves old r2 for address adds */
    0x245B, /* or r5,r4 */
    0x2459, /* and r5,r4 */
    0x245A, /* xor r5,r4 */
    0x3450, /* cmp/eq r5,r4 */
    0x4D11  /* cmp/pz r13 */
  };

  reset_buffer();
  SH4_EMIT_ADD(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_ADD(sh4_reg_r2, sh4_reg_r12, sh4_reg_r2);
  SH4_EMIT_OR(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_AND(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_XOR(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_CMP_REG(sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_CMP_PZ(sh4_reg_r13);
  expect_words("alu and compare encodings", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_shift_and_call_encodings(void)
{
  const u16 expected[] = {
    0x4300, /* shll r3 */
    0x4301, /* shlr r3 */
    0x4321, /* shar r3 */
    0x4305, /* rotr r3 */
    0x4318, /* shll8 r3 */
    0x410B, /* jsr @r1 */
    0x0009
  };

  reset_buffer();
  SH4_EMIT_SHLL1(sh4_reg_r3);
  SH4_EMIT_SHLR1(sh4_reg_r3);
  SH4_EMIT_SHAR1(sh4_reg_r3);
  SH4_EMIT_ROTR1(sh4_reg_r3);
  SH4_EMIT_SHLL8(sh4_reg_r3);
  SH4_EMIT_JSR(sh4_reg_r1);
  expect_words("shift and call encodings", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_branch_filler_polarity(void)
{
  u8 *patch;
  /* Each conditional skip is now tst/cmp + short bt/bf hop + 12-bit bra + nop.
     true:  skip (bra) when T==1 -> hop is bf .+6 (0x8B01)
     false: skip (bra) when T==0 -> hop is bt .+6 (0x8901) */
  const u16 expected[] = {
    0x2448, 0x8B01, 0xA000, 0x0009, /* true  */
    0x2448, 0x8901, 0xA000, 0x0009  /* false */
  };

  reset_buffer();
  generate_branch_filler_true(a0, a1, patch);
  generate_branch_filler_false(a0, a1, patch);
  (void)patch;
  expect_words("boolean branch filler polarity", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_conditional_skip_patch(void)
{
  u8 *skip;
  u8 *near_target;
  u16 expected_bra;

  /* The writeback location points at the bra; patching reaches far targets a
     lone bt/bf (+/-254 bytes) could not. */
  reset_buffer();
  generate_branch_filler_true(a0, a1, skip);
  near_target = (u8 *)translation_ptr;
  expected_bra = 0xA000 |
   (sh4_relative_offset_words(skip, near_target) & 0x0FFF);

  generate_branch_patch_conditional(skip, near_target);
  if(*((u16 *)skip) != expected_bra)
  {
    printf("conditional skip near patch failed: got %04x expected %04x\n",
     *((u16 *)skip), expected_bra);
    failures++;
  }
  else
  {
    printf("conditional skip near patch: ok\n");
  }

  /* A far skip (beyond the old 8-bit reach) still encodes a valid bra. */
  reset_buffer();
  generate_branch_filler_equal(a0, a1, skip);
  {
    u8 *far_target = skip + 2000; /* ~1000 words, far past 8-bit bt/bf range */
    s32 off = sh4_relative_offset_words(skip, far_target);
    expected_bra = 0xA000 | (off & 0x0FFF);
    generate_branch_patch_conditional(skip, far_target);
    if(*((u16 *)skip) != expected_bra || off < -2048 || off > 2047)
    {
      printf("conditional skip far patch failed: got %04x expected %04x "
       "off=%d\n", *((u16 *)skip), expected_bra, off);
      failures++;
    }
    else
    {
      printf("conditional skip far patch: ok\n");
    }
  }
}

static void test_far_conditional_skip(void)
{
  u8 *skip;
  u8 *near_target;
  u8 *far_target;
  u8 *veneer_end;
  u32 *literal;
  u32 literal_value;
  u16 hop_word;
  u16 expected_hop;
  u16 expected_bra;

  /* Shape: bf hop + nop, then an absolute-jump slot the hop lands past. */
  reset_buffer();
  SH4_EMIT_COND_SKIP_T_FAR(skip);
  veneer_end = (u8 *)translation_ptr;
  hop_word = code_buffer[0];
  expected_hop = 0x8B00 |
   (sh4_relative_offset_words(code_buffer, veneer_end) & 0xFF);

  if(hop_word != expected_hop || code_buffer[1] != 0x0009 ||
   ((u16 *)skip)[0] != 0xD101 || ((u16 *)skip)[1] != 0x412B)
  {
    printf("far conditional skip shape failed: hop=%04x expected=%04x "
     "slot=%04x %04x\n", hop_word, expected_hop, ((u16 *)skip)[0],
     ((u16 *)skip)[1]);
    failures++;
  }
  else
  {
    printf("far conditional skip shape: ok\n");
  }

  /* Polarity: SKIP_F hops with bt instead of bf. */
  reset_buffer();
  SH4_EMIT_COND_SKIP_F_FAR(skip);
  if((code_buffer[0] & 0xFF00) != 0x8900)
  {
    printf("far conditional skip polarity failed: hop=%04x\n", code_buffer[0]);
    failures++;
  }
  else
  {
    printf("far conditional skip polarity: ok\n");
  }

  /* Near patch through the conditional dispatcher rewrites the veneer to
     bra/nop. */
  reset_buffer();
  SH4_EMIT_COND_SKIP_T_FAR(skip);
  SH4_EMIT_NOP();
  near_target = (u8 *)translation_ptr;
  expected_bra = 0xA000 |
   (sh4_relative_offset_words(skip, near_target) & 0x0FFF);
  generate_branch_patch_conditional(skip, near_target);
  if(((u16 *)skip)[0] != expected_bra || ((u16 *)skip)[1] != 0x0009)
  {
    printf("far conditional skip near patch failed: got %04x %04x\n",
     ((u16 *)skip)[0], ((u16 *)skip)[1]);
    failures++;
  }
  else
  {
    printf("far conditional skip near patch: ok\n");
  }

  /* Far patch keeps the veneer and fills its literal: this is the case the
     12-bit bra slot could not represent. */
  reset_buffer();
  SH4_EMIT_COND_SKIP_T_FAR(skip);
  far_target = skip + 0x10000;
  generate_branch_patch_conditional(skip, far_target);
  literal = sh4_long_branch_literal(skip);
  memcpy(&literal_value, literal, sizeof(literal_value));
  if(((u16 *)skip)[0] != 0xD101 ||
   literal_value != (u32)(unsigned long)far_target)
  {
    printf("far conditional skip far patch failed: %04x literal=%08x\n",
     ((u16 *)skip)[0], literal_value);
    failures++;
  }
  else
  {
    printf("far conditional skip far patch: ok\n");
  }
}

/* Minimal translate-loop context so arm_conditional_block_header() can be
   expanded as-is: the header scans block_data for the same-condition run
   and picks the near or far skip shape. */
#define REG_N_FLAG 16
#define REG_Z_FLAG 17
#define REG_C_FLAG 18
#define REG_V_FLAG 19
#define arm_instruction_width 4

typedef struct
{
  u8 condition;
} test_block_data_type;

static test_block_data_type block_data[64];

static void test_conditional_header_run_selection(void)
{
  u32 condition = 0x01; /* NE */
  u32 last_condition = 0x01;
  s32 block_data_position = 0;
  u32 pc = 0x08000000;
  u32 block_end_pc = pc + (64 * arm_instruction_width);
  u8 *backpatch_address = (u8 *)0;
  int i;

  (void)condition;

  /* Short run (2 instructions): worst-case estimate fits the 12-bit bra,
     so the skip slot must be the near bra filler. */
  for(i = 0; i < 64; i++)
    block_data[i].condition = 0x0E;
  block_data[0].condition = 0x01;
  block_data[1].condition = 0x01;

  reset_buffer();
  {
    condition_check_type condition_check;
    arm_conditional_block_header();
  }

  if(backpatch_address == NULL ||
   (*((u16 *)backpatch_address) & 0xF000) != 0xA000)
  {
    printf("conditional header near selection failed: slot=%04x\n",
     backpatch_address ? *((u16 *)backpatch_address) : 0);
    failures++;
  }
  else
  {
    printf("conditional header near selection: ok\n");
  }

  /* Long run (20 instructions): the estimate exceeds the bra reach, so the
     header must reserve the far absolute-jump slot instead. */
  for(i = 0; i < 20; i++)
    block_data[i].condition = 0x01;

  backpatch_address = (u8 *)0;
  reset_buffer();
  {
    condition_check_type condition_check;
    arm_conditional_block_header();
  }

  if(backpatch_address == NULL || ((u16 *)backpatch_address)[0] != 0xD101 ||
   ((u16 *)backpatch_address)[1] != 0x412B)
  {
    printf("conditional header far selection failed: slot=%04x %04x\n",
     backpatch_address ? ((u16 *)backpatch_address)[0] : 0,
     backpatch_address ? ((u16 *)backpatch_address)[1] : 0);
    failures++;
  }
  else
  {
    printf("conditional header far selection: ok\n");
  }
}

static void test_conditional_skip_range_backstop(void)
{
  u8 *skip;
  u32 fatal_before = fatal_error_count;

  /* Patching a short bra slot with an unreachable target must fail loudly
     instead of emitting a silently truncated branch. */
  reset_buffer();
  generate_branch_filler_true(a0, a1, skip);
  generate_branch_patch_conditional(skip, skip + 0x4000);

  if(fatal_error_count != fatal_before + 1)
  {
    printf("conditional skip range backstop failed: no fatal error\n");
    failures++;
  }
  else
  {
    printf("conditional skip range backstop: ok\n");
  }
}

static void test_cycle_update_guard(void)
{
  u32 cycle_count = 0;

  /* A zero cycle balance must not emit a load/sub pair. */
  reset_buffer();
  generate_cycle_update();
  if(translation_ptr != code_buffer)
  {
    printf("cycle update guard failed: emitted %zu words for zero count\n",
     (size_t)(translation_ptr - code_buffer));
    failures++;
    return;
  }

  cycle_count = 3;
  generate_cycle_update();
  if(translation_ptr == code_buffer || cycle_count != 0)
  {
    printf("cycle update guard failed: nonzero count emitted nothing\n");
    failures++;
    return;
  }

  printf("cycle update guard: ok\n");
}

static void test_load_imm_encodings(void)
{
  const u16 expected[] = {
    0xE47F,                         /* mov #0x7f,r4 */
    0xE480,                         /* mov #-0x80,r4 */
    0xE480, 0x644C,                 /* 0x00000080: mov/extu.b */
    0xE412, 0x4418, 0x7434,         /* 0x12345678 */
    0x4418, 0x7456, 0x4418, 0x7478,
    0xE408, 0x4418, 0x4418,         /* 0x08000068 */
    0x4418, 0x7468
  };

  reset_buffer();
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x0000007F);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0xFFFFFF80);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x00000080);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x12345678);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x08000068);
  expect_words("load immediate encodings", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_branch_patch_and_veneer(void)
{
  u8 *branch;
  u8 *target;
  u32 *literal;
  u32 literal_value;
  size_t emitted;
  size_t literal_index;
  size_t expected_count;
  u16 expected_branch;

  reset_buffer();
  SH4_EMIT_BRA_FILLER(branch);
  SH4_EMIT_NOP();
  SH4_EMIT_NOP();
  target = (u8 *)translation_ptr;
  expected_branch = 0xA000 |
   (sh4_relative_offset_words(branch, target) & 0x0FFF);

  if(!sh4_branch12_in_range(branch, target))
  {
    printf("near branch range failed\n");
    failures++;
  }

  generate_branch_patch_unconditional_direct(branch, target);
  if(code_buffer[0] != expected_branch)
  {
    printf("branch patch failed: got %04x expected %04x\n",
     code_buffer[0], expected_branch);
    failures++;
  }
  else
  {
    printf("branch patch: ok\n");
  }

  if(sh4_branch12_in_range((u8 *)code_buffer, (u8 *)code_buffer + 0x4000))
  {
    printf("far branch range failed\n");
    failures++;
  }
  else
  {
    printf("branch range limits: ok\n");
  }

  reset_buffer();
  SH4_EMIT_ABSOLUTE_JUMP_VENEER(0x8C123456, literal);
  emitted = (size_t)(translation_ptr - code_buffer);
  literal_index = (size_t)((u16 *)literal - code_buffer);
  expected_count = ((((uintptr_t)code_buffer + 6) & 3) != 0) ? 6 : 5;
  memcpy(&literal_value, literal, sizeof(literal_value));

  if(emitted != expected_count || literal_index != expected_count - 2 ||
   code_buffer[0] != 0xD101 || code_buffer[1] != 0x412B ||
   code_buffer[2] != 0x0009 || (expected_count == 6 &&
   code_buffer[3] != 0x0009) || literal_value != 0x8C123456)
  {
    printf("absolute jump veneer failed: emitted=%zu literal_index=%zu "
     "literal=%08x\n", emitted, literal_index, literal_value);
    failures++;
  }
  else
  {
    printf("absolute jump veneer: ok\n");
  }
}

static void test_long_branch_filler_patch(void)
{
  u8 *branch;
  u8 *near_target;
  u8 *far_target;
  u32 *literal;
  u32 literal_value;
  u16 expected_branch;

  /* Near target: the slot is rewritten to bra/nop; the veneer's jmp must
     not survive in the bra delay slot. */
  reset_buffer();
  SH4_EMIT_LONG_BRANCH_FILLER(branch);
  SH4_EMIT_NOP();
  near_target = (u8 *)translation_ptr;
  expected_branch = 0xA000 |
   (sh4_relative_offset_words(branch, near_target) & 0x0FFF);

  generate_branch_patch_unconditional(branch, near_target);
  if(((u16 *)branch)[0] != expected_branch || ((u16 *)branch)[1] != 0x0009)
  {
    printf("near long branch patch failed: got %04x %04x expected %04x 0009\n",
     ((u16 *)branch)[0], ((u16 *)branch)[1], expected_branch);
    failures++;
  }
  else
  {
    printf("near long branch patch: ok\n");
  }

  /* Far target: the veneer instructions stay and the literal receives the
     absolute target address. */
  reset_buffer();
  SH4_EMIT_LONG_BRANCH_FILLER(branch);
  far_target = branch + 0x10000;

  generate_branch_patch_unconditional(branch, far_target);
  literal = sh4_long_branch_literal(branch);
  memcpy(&literal_value, literal, sizeof(literal_value));
  if(((u16 *)branch)[0] != 0xD101 || ((u16 *)branch)[1] != 0x412B ||
   ((u16 *)branch)[2] != 0x0009 ||
   literal_value != (u32)(unsigned long)far_target)
  {
    printf("far long branch patch failed: %04x %04x %04x literal=%08x\n",
     ((u16 *)branch)[0], ((u16 *)branch)[1], ((u16 *)branch)[2],
     literal_value);
    failures++;
  }
  else
  {
    printf("far long branch patch: ok\n");
  }
}

static void test_icache_range_hook(void)
{
  u8 cache[64];
  u8 *cache_end = cache + 28;
  u32 expected_addr = (u32)(uintptr_t)cache;
  u32 expected_size = 28 + 0x100;

  invalidate_count = 0;
  last_invalidate_addr = 0;
  last_invalidate_size = 0;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif
  translate_invalidate_dcache_region(cache, cache_end);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  if(invalidate_count != 1 || last_invalidate_addr != expected_addr ||
   last_invalidate_size != expected_size)
  {
    printf("icache range hook failed: count=%u addr=%08x size=%u\n",
     invalidate_count, last_invalidate_addr, last_invalidate_size);
    failures++;
  }
  else
  {
    printf("icache range hook: ok\n");
  }
}

#include "sh4_emit_simulator.h"

int main(void)
{
  test_executed_emission();
  test_load_store_encodings();
  test_alu_encodings();
  test_shift_and_call_encodings();
  test_branch_filler_polarity();
  test_conditional_skip_patch();
  test_far_conditional_skip();
  test_conditional_header_run_selection();
  test_conditional_skip_range_backstop();
  test_cycle_update_guard();
  test_load_imm_encodings();
  test_branch_patch_and_veneer();
  test_long_branch_filler_patch();
  test_icache_range_hook();

  return failures == 0 ? 0 : 1;
}
