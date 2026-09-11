/* Execute the production SH-4 C helpers, with host stubs for memory/IRQ I/O. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define COMMON_H
#define function_cc
#define file_tag_type FILE *
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
#include "../cpu.h"
#define REG_IE 0
#define REG_IF 1
#define REG_IME 2
#define address32(base, offset) (*(u32 *)((u8 *)(base) + (offset)))
#define ror(dest, value, shift) dest = ((value) >> (shift)) | ((value) << (32 - (shift)))
u32 reg[64], reg_mode[7][7], spsr[6], cpu_modes[32], bios_read_protect;
u16 io_registers[3];
u8 *memory_map_read[8192], *memory_map_write[8192];
const u8 bit_count[256] = {0};
u32 read_memory8(u32 address) { return 0; }
u32 read_memory16(u32 address) { return 0; }
u32 read_memory16_signed(u32 address) { return 0; }
u32 read_memory32(u32 address) { return 0; }
cpu_alert_type write_memory8(u32 a, u8 v) { return CPU_ALERT_NONE; }
cpu_alert_type write_memory16(u32 a, u16 v) { return CPU_ALERT_NONE; }
cpu_alert_type write_memory32(u32 a, u32 v) { return CPU_ALERT_NONE; }
void set_cpu_mode(u32 mode) { reg[CPU_MODE] = mode; }
#include "../dc/sh4_helpers.c"

static unsigned failures, checks;
static void check(int pass, const char *name, u32 a, u32 b, u32 carry)
{
  checks++;
  if(!pass && failures++ < 12)
    printf("%s failed: a=%08x b=%08x carry=%u\n", name, a, b, carry);
}

static void arithmetic(u32 a, u32 b, u32 carry)
{
  u32 result, expected, c, v;
  unsigned op;
  for(op = 0; op < 6; op++)
  {
    u32 lhs = op >= 4 ? b : a;
    u32 rhs = op >= 4 ? a : b;
    u32 extra = (op & 1) ? (op < 2 ? carry : 1 - carry) : 0;
    s64 signed_result = op < 2 ? (s64)(s32)lhs + (s32)rhs + extra :
     (s64)(s32)lhs - (s32)rhs - extra;
    u64 wide = op < 2 ? (u64)lhs + rhs + extra : (u64)rhs + extra;
    expected = (u32)signed_result;
    c = op < 2 ? (u32)(wide >> 32) : (u64)lhs >= wide;
    v = signed_result > INT32_MAX || signed_result < INT32_MIN;
    reg[REG_C_FLAG] = carry;
    switch(op)
    {
      case 0: result = execute_adds(b, a); break;
      case 1: result = execute_adcs(b, a); break;
      case 2: result = execute_subs(b, a); break;
      case 3: result = execute_sbcs(b, a); break;
      case 4: result = execute_rsbs(b, a); break;
      default: result = execute_rscs(b, a); break;
    }
    check(result == expected && reg[REG_C_FLAG] == c &&
     reg[REG_V_FLAG] == v && reg[REG_Z_FLAG] == (expected == 0) &&
     reg[REG_N_FLAG] == (expected >> 31), "arithmetic NZCV", a, b, carry);
  }
}

static void shifts(u32 value, u32 amount, u32 carry)
{
  u32 type, variant;
  for(type = 0; type < 4; type++)
  {
    u32 expected = value, c = carry, i;
    /* Bit-by-bit reference avoids host shifts by 32 or more. */
    for(i = 0; i < (amount & 255); i++)
    {
      c = type == 0 ? expected >> 31 : expected & 1;
      if(type == 0) expected <<= 1;
      if(type == 1) expected >>= 1;
      if(type == 2) expected = (expected >> 1) | (expected & 0x80000000);
      if(type == 3) expected = (expected >> 1) | (c << 31);
    }
    for(variant = 0; variant < 3; variant++)
    {
      u32 result;
      reg[REG_C_FLAG] = carry;
      reg[REG_N_FLAG] = reg[REG_Z_FLAG] = reg[REG_V_FLAG] = 1;
      if(variant == 0)
      {
        switch(type) {
          case 0: result = execute_lsl_no_flags_reg(value, amount); break;
          case 1: result = execute_lsr_no_flags_reg(value, amount); break;
          case 2: result = execute_asr_no_flags_reg(value, amount); break;
          default: result = execute_ror_no_flags_reg(value, amount); break;
        }
      }
      else if(variant == 1)
      {
        switch(type) {
          case 0: result = execute_lsl_flags_reg(value, amount); break;
          case 1: result = execute_lsr_flags_reg(value, amount); break;
          case 2: result = execute_asr_flags_reg(value, amount); break;
          default: result = execute_ror_flags_reg(value, amount); break;
        }
      }
      else
      {
        switch(type) {
          case 0: result = execute_lsl_reg_op(value, amount); break;
          case 1: result = execute_lsr_reg_op(value, amount); break;
          case 2: result = execute_asr_reg_op(value, amount); break;
          default: result = execute_ror_reg_op(value, amount); break;
        }
      }
      check(result == expected && reg[REG_C_FLAG] == (variant ? c : carry) &&
       reg[REG_V_FLAG] == 1 &&
       reg[REG_N_FLAG] == (variant == 2 ? expected >> 31 : 1) &&
       reg[REG_Z_FLAG] == (variant == 2 ? expected == 0 : 1),
       "register shift", value, amount, carry);
    }
  }
}

static void cpsr_masked_write(void)
{
  u32 result;
  memset(reg, 0, sizeof(reg));
  memset(io_registers, 0, sizeof(io_registers));
  cpu_modes[0x13] = MODE_SUPERVISOR;
  reg[REG_CPSR] = 0x10;
  reg[REG_N_FLAG] = reg[REG_C_FLAG] = 1;
  result = execute_store_cpsr(0x13, 0xFF, 0x08000000);
  check(result == 0 && reg[REG_CPSR] == 0xA0000013 &&
   reg[REG_N_FLAG] == 1 && reg[REG_C_FLAG] == 1,
   "MSR control preserves live NZCV", 0, 0, 0);
  result = execute_store_cpsr(0x50000000, 0xF0000000, 0x08000000);
  check(result == 0 && reg[REG_CPSR] == 0x50000013 &&
   reg[REG_N_FLAG] == 0 && reg[REG_Z_FLAG] == 1 &&
   reg[REG_C_FLAG] == 0 && reg[REG_V_FLAG] == 1,
   "MSR flags replace live NZCV", 0, 0, 0);
  reg[REG_CPSR] = 0x93;
  reg[REG_N_FLAG] = reg[REG_C_FLAG] = 1;
  reg[REG_Z_FLAG] = reg[REG_V_FLAG] = 0;
  io_registers[REG_IE] = io_registers[REG_IF] = io_registers[REG_IME] = 1;
  result = execute_store_cpsr(0x13, 0xFF, 0x08000000);
  check(result == 0x18 && spsr[MODE_IRQ] == 0xA0000013,
   "MSR IRQ snapshots live NZCV", 0, 0, 0);
}

static void exception_return(void)
{
  u32 thumb, low, irq;
  cpu_modes[0x10] = MODE_USER;
  for(thumb = 0; thumb < 2; thumb++)
    for(low = 0; low < 4; low++)
      for(irq = 0; irq < 2; irq++)
      {
        u32 target = 0x08000100 | low;
        u32 aligned = target & (thumb ? ~1u : ~3u);
        u32 expected = irq ? 0x18 : aligned | thumb;
        u32 result;
        memset(reg, 0, sizeof(reg));
        reg[CPU_MODE] = MODE_SUPERVISOR;
        spsr[MODE_SUPERVISOR] = 0xA0000010 | (thumb << 5);
        io_registers[REG_IE] = io_registers[REG_IF] = io_registers[REG_IME] = irq;
        result = execute_spsr_restore(target);
        check(result == expected && reg[REG_N_FLAG] == 1 &&
         reg[REG_C_FLAG] == 1 &&
         (!irq || reg_mode[MODE_IRQ][6] == aligned + 4),
         "exception-return alignment/state", target, thumb, irq);
      }
}

int main(void)
{
  static const u32 edge[] = {0, 1, 0x7fffffff, 0x80000000,
   0xfffffffe, 0xffffffff, 0x55555555, 0xaaaaaaaa};
  unsigned a, b, carry;
  u32 random = 0x12345678;
  for(a = 0; a < sizeof(edge) / sizeof(edge[0]); a++)
    for(carry = 0; carry < 2; carry++)
    {
      for(b = 0; b < sizeof(edge) / sizeof(edge[0]); b++)
        arithmetic(edge[a], edge[b], carry);
      for(b = 0; b < 512; b++) shifts(edge[a], b, carry);
      shifts(edge[a], 0xffffffff, carry);
    }
  for(a = 0; a < 10000; a++)
  {
    u32 lhs = random;
    random = random * 1664525u + 1013904223u;
    arithmetic(lhs, random, a & 1);
  }
  cpsr_masked_write();
  exception_return();
  printf("SH-4 helpers: %u checks, %u failures\n", checks, failures);
  return failures != 0;
}
