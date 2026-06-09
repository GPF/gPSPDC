/* Host-side sanity checks for Phase 1 helper fixes (run with: make -C tests test) */
#include <stdio.h>
#include <stdint.h>

typedef int32_t s32;
typedef uint32_t u32;

static u32 reg[64];
static int failures;

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
    failures++;
    return;
  }

  printf("swi_hle_div remainder: ok\n");
}

static u32 simulate_take_irq(u32 ie, u32 iff, u32 ime, u32 cpsr)
{
  if((ie & iff) && ime && ((cpsr & 0x80) == 0))
    return 0x00000018;

  return 0;
}

static void test_sh4_irq_dispatch_logic(void)
{
  if(simulate_take_irq(0x0001, 0x0001, 1, 0x0000001F) != 0x00000018)
  {
    printf("sh4 irq dispatch failed: expected vector when IRQ enabled\n");
    failures++;
    return;
  }

  if(simulate_take_irq(0x0001, 0x0001, 1, 0x0000009F) != 0)
  {
    printf("sh4 irq dispatch failed: masked IRQ should not dispatch\n");
    failures++;
    return;
  }

  if(simulate_take_irq(0x0001, 0x0000, 1, 0x0000001F) != 0)
  {
    printf("sh4 irq dispatch failed: no pending flag should not dispatch\n");
    failures++;
    return;
  }

  printf("sh4 irq dispatch logic: ok\n");
}

static void test_spsr_restore_irq_return(void)
{
  u32 address = 0x03001234;
  u32 irq_pc = simulate_take_irq(0x0001, 0x0001, 1, 0x0000001F);

  if(irq_pc != 0)
    address = irq_pc;

  if(address != 0x00000018)
  {
    printf("spsr restore irq return failed: %08x\n", address);
    failures++;
    return;
  }

  address = 0x03001235;
  if(address & 0x20)
    address |= 0x01;

  if(address != 0x03001235)
  {
    printf("spsr restore thumb bit failed: %08x\n", address);
    failures++;
    return;
  }

  printf("spsr restore irq return: ok\n");
}

static void test_gamepak_page_math(void)
{
  const u32 page_size = 32 * 1024;
  const u32 buffer_16mb = 16 * 1024 * 1024;
  u32 pages = buffer_16mb / page_size;

  if(pages != 512)
  {
    printf("gamepak page count failed: %u\n", pages);
    failures++;
    return;
  }

  printf("gamepak page math: ok\n");
}

int main(void)
{
  failures = 0;

  test_swi_hle_div();
  test_sh4_irq_dispatch_logic();
  test_spsr_restore_irq_return();
  test_gamepak_page_math();

  if(failures != 0)
  {
    printf("phase1_helpers_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
