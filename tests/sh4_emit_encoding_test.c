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
    0x3450  /* cmp/eq r5,r4 */
  };

  reset_buffer();
  SH4_EMIT_ADD(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_ADD(sh4_reg_r2, sh4_reg_r12, sh4_reg_r2);
  SH4_EMIT_OR(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_AND(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_XOR(sh4_reg_r4, sh4_reg_r4, sh4_reg_r5);
  SH4_EMIT_CMP_REG(sh4_reg_r4, sh4_reg_r5);
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
  translation_ptr_t patch;
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
    0x4418, 0x7456, 0x4418, 0x7478
  };

  reset_buffer();
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x0000007F);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0xFFFFFF80);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x00000080);
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, 0x12345678);
  expect_words("load immediate encodings", expected,
   sizeof(expected) / sizeof(expected[0]));
}

int main(void)
{
  test_load_store_encodings();
  test_alu_encodings();
  test_shift_and_call_encodings();
  test_branch_filler_polarity();
  test_load_imm_encodings();

  return failures == 0 ? 0 : 1;
}
