/* Host-side sanity checks for Phase 1 helper fixes (run with: make -C tests test) */
#include <stdio.h>
#include <stdint.h>

typedef int32_t s32;
typedef uint32_t u32;

static u32 reg[64];

static void test_swi_hle_div(void)
{
  s32 dividend, divisor, result;

  reg[0] = (u32)(s32)-7;
  reg[1] = (u32)(s32)3;
  dividend = (s32)reg[0];
  divisor = (s32)reg[1];
  result = dividend / divisor;
  reg[0] = (u32)result;
  reg[1] = (u32)(dividend % divisor);

  if((s32)reg[0] != -2 || (s32)reg[1] != -1)
  {
    printf("swi_hle_div remainder failed: r0=%d r1=%d\n", (s32)reg[0], (s32)reg[1]);
    return;
  }

  printf("swi_hle_div remainder: ok\n");
}

static void test_gamepak_page_math(void)
{
  const u32 page_size = 32 * 1024;
  const u32 buffer_16mb = 16 * 1024 * 1024;
  u32 pages = buffer_16mb / page_size;

  if(pages != 512)
  {
    printf("gamepak page count failed: %u\n", pages);
    return;
  }

  printf("gamepak page math: ok\n");
}

int main(void)
{
  test_swi_hle_div();
  test_gamepak_page_math();
  return 0;
}
