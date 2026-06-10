#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define COMMON_H
#define CPU_H
#define function_cc

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
  const u16 expected[] = {
    0x2448, 0x8900, 0x0009, /* true: skip when tested value is zero */
    0x2448, 0x8B00, 0x0009  /* false: skip when tested value is non-zero */
  };

  reset_buffer();
  generate_branch_filler_true(a0, a1, patch);
  generate_branch_filler_false(a0, a1, patch);
  (void)patch;
  expect_words("boolean branch filler polarity", expected,
   sizeof(expected) / sizeof(expected[0]));
}

static void test_load_imm_encodings(void)
{
  const u16 expected[] = {
    0xE47F,                         /* mov #0x7f,r4 */
    0xE480,                         /* mov #-0x80,r4 */
    0xE400, 0x4418, 0x4418, 0x4418, /* 0x00000080 */
    0x747F, 0x7401,
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

int main(void)
{
  test_load_store_encodings();
  test_alu_encodings();
  test_shift_and_call_encodings();
  test_branch_filler_polarity();
  test_load_imm_encodings();
  test_branch_patch_and_veneer();
  test_long_branch_filler_patch();
  test_icache_range_hook();

  return failures == 0 ? 0 : 1;
}
