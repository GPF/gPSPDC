/* SH-4 dynarec helper functions (ported from x86/x86_emit.h) */
#include <kos.h>
#include "common.h"
#include "cpu.h"

extern u32 reg[];
extern u32 reg_mode[7][7];
extern u32 spsr[];
extern u8 *memory_map_read[];
extern u8 *memory_map_write[];
extern const u8 bit_count[256];

extern u32 read_memory8(u32 address);
extern u32 read_memory16(u32 address);
extern u32 read_memory32(u32 address);
extern s32 read_memory16_signed(u32 address);
extern cpu_alert_type write_memory8(u32 address, u8 value);
extern cpu_alert_type write_memory16(u32 address, u16 value);
extern cpu_alert_type write_memory32(u32 address, u32 value);
extern void set_cpu_mode(u32 mode);
extern u32 cpu_modes[32];
extern u16 io_registers[];

#define calculate_z_flag(dest) (reg[REG_Z_FLAG] = ((dest) == 0))
#define calculate_n_flag(dest) (reg[REG_N_FLAG] = ((signed)(dest) < 0))
#define calculate_c_flag_sub(dest, src_a, src_b) (reg[REG_C_FLAG] = ((unsigned)(src_b) <= (unsigned)(src_a)))
#define calculate_v_flag_sub(dest, src_a, src_b) (reg[REG_V_FLAG] = ((signed)(src_b) > (signed)(src_a)) != ((signed)(dest) < 0))
#define calculate_c_flag_add(dest, src_a, src_b) (reg[REG_C_FLAG] = ((unsigned)(dest) < (unsigned)(src_a)))
#define calculate_v_flag_add(dest, src_a, src_b) (reg[REG_V_FLAG] = ((signed)(dest) < (signed)(src_a)) != ((signed)(src_b) < 0))
#define calculate_flags_add(dest, src_a, src_b) do { calculate_z_flag(dest); calculate_n_flag(dest); calculate_c_flag_add(dest, src_a, src_b); calculate_v_flag_add(dest, src_a, src_b); } while(0)
#define calculate_flags_sub(dest, src_a, src_b) do { calculate_z_flag(dest); calculate_n_flag(dest); calculate_c_flag_sub(dest, src_a, src_b); calculate_v_flag_sub(dest, src_a, src_b); } while(0)
#define calculate_flags_logic(dest) do { calculate_z_flag(dest); calculate_n_flag(dest); } while(0)

#define collapse_flags() \
  reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) | (reg[REG_Z_FLAG] << 30) | \
   (reg[REG_C_FLAG] << 29) | (reg[REG_V_FLAG] << 28) | \
   (reg[REG_CPSR] & 0xFF)

#define extract_flags() do { \
  reg[REG_N_FLAG] = reg[REG_CPSR] >> 31; \
  reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 0x01; \
  reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 0x01; \
  reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 0x01; \
} while(0)

#define check_for_interrupts() do { \
  u32 address; \
  if((io_registers[REG_IE] & io_registers[REG_IF]) && \
   io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0)) { \
    reg_mode[MODE_IRQ][6] = reg[REG_PC] + 4; \
    spsr[MODE_IRQ] = reg[REG_CPSR]; \
    reg[REG_CPSR] = 0xD2; \
    address = 0x00000018; \
    set_cpu_mode(MODE_IRQ); \
  } \
} while(0)


u32 function_cc execute_and(u32 rm, u32 rn) { return rn & rm; }
u32 function_cc execute_ands(u32 rm, u32 rn) { u32 dest = rn & rm; calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_eor(u32 rm, u32 rn) { return rn ^ rm; }
u32 function_cc execute_eors(u32 rm, u32 rn) { u32 dest = rn ^ rm; calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_orr(u32 rm, u32 rn) { return rn | rm; }
u32 function_cc execute_orrs(u32 rm, u32 rn) { u32 dest = rn | rm; calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_bic(u32 rm, u32 rn) { return rn & (~rm); }
u32 function_cc execute_bics(u32 rm, u32 rn) { u32 dest = rn & (~rm); calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_mul(u32 rm, u32 rn) { return rn * rm; }
u32 function_cc execute_mov(u32 rm) { return rm; }
u32 function_cc execute_movs(u32 rm) { u32 dest = rm; calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_mvn(u32 rm) { return ~rm; }
u32 function_cc execute_mvns(u32 rm) { u32 dest = ~rm; calculate_z_flag(dest); calculate_n_flag(dest); return dest; }
u32 function_cc execute_sub(u32 rm, u32 rn) { return rn - rm; }
u32 function_cc execute_subs(u32 rm, u32 rn) { u32 dest = rn - rm; calculate_flags_sub(dest, rn, rm); return dest; }
u32 function_cc execute_rsb(u32 rm, u32 rn) { return rm - rn; }
u32 function_cc execute_rsbs(u32 rm, u32 rn) { u32 dest = rm - rn; calculate_flags_sub(dest, rm, rn); return dest; }
u32 function_cc execute_sbc(u32 rm, u32 rn) { return rn - rm - (reg[REG_C_FLAG] ^ 1); }
u32 function_cc execute_sbcs(u32 rm, u32 rn) { u32 dest = rn - rm - (reg[REG_C_FLAG] ^ 1); calculate_flags_sub(dest, rn, rm); return dest; }
u32 function_cc execute_rsc(u32 rm, u32 rn) { return rm + reg[REG_C_FLAG] - 1 - rn; }
u32 function_cc execute_rscs(u32 rm, u32 rn) { u32 dest = rm + reg[REG_C_FLAG] - 1 - rn; calculate_flags_sub(dest, rm, rn); return dest; }
u32 function_cc execute_add(u32 rm, u32 rn) { return rn + rm; }
u32 function_cc execute_adds(u32 rm, u32 rn) { u32 dest = rn + rm; calculate_flags_add(dest, rn, rm); return dest; }
u32 function_cc execute_adc(u32 rm, u32 rn) { return rn + rm + reg[REG_C_FLAG]; }
u32 function_cc execute_adcs(u32 rm, u32 rn) { u32 dest = rn + rm + reg[REG_C_FLAG]; calculate_flags_add(dest, rn, rm); return dest; }
void function_cc execute_tst(u32 rm, u32 rn) { u32 dest = rn & rm; calculate_z_flag(dest); calculate_n_flag(dest); }
void function_cc execute_teq(u32 rm, u32 rn) { u32 dest = rn ^ rm; calculate_z_flag(dest); calculate_n_flag(dest); }
void function_cc execute_cmp(u32 rm, u32 rn) { u32 dest = rn - rm; calculate_flags_sub(dest, rn, rm); }
void function_cc execute_cmn(u32 rm, u32 rn) { u32 dest = rn + rm; calculate_flags_add(dest, rn, rm); }
u32 function_cc execute_lsl_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = 0;
    else
      value <<= shift;
  }
  return value;
}
u32 function_cc execute_lsr_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = 0;
    else
      value >>= shift;
  }
  return value;
}
u32 function_cc execute_asr_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
      value = (s32)value >> 31;
    else
      value = (s32)value >> shift;
  }
  return value;
}
u32 function_cc execute_ror_no_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    ror(value, value, shift);
  }

  return value;
}
u32 function_cc execute_lsl_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = value & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
      value <<= shift;
    }
  }
  return value;
}
u32 function_cc execute_lsr_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = (value >> 31) & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value >>= shift;
    }
  }
  return value;
}
u32 function_cc execute_asr_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      value = (s32)value >> 31;
      reg[REG_C_FLAG] = value & 0x01;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value = (s32)value >> shift;
    }
  }
  return value;
}
u32 function_cc execute_ror_flags_reg(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }

  return value;
}
u32 function_cc execute_rrx_flags(u32 value)
{
  u32 c_flag = reg[REG_C_FLAG];
  reg[REG_C_FLAG] = value & 0x01;
  return (value >> 1) | (c_flag << 31);
}
u32 function_cc execute_rrx(u32 value)
{
  return (value >> 1) | (reg[REG_C_FLAG] << 31);
}
u32 function_cc execute_spsr_restore(u32 address)
{
  reg[REG_CPSR] = spsr[reg[CPU_MODE]];
  extract_flags();
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0x1F]);
  check_for_interrupts();

  if(reg[REG_CPSR] & 0x20)
    address |= 0x01;

  return address;
}
u32 function_cc execute_mul_flags(u32 dest)
{
  calculate_z_flag(dest);
  calculate_n_flag(dest);
}
u32 function_cc execute_mul_long_flags(u32 dest_lo, u32 dest_hi)
{
  reg[REG_Z_FLAG] = (dest_lo == 0) & (dest_hi == 0);
  calculate_n_flag(dest_hi);
}
u32 function_cc execute_read_cpsr()
{
  collapse_flags();
  return reg[REG_CPSR];
}
u32 function_cc execute_read_spsr()
{
  collapse_flags();
  return spsr[reg[CPU_MODE]];
}
void function_cc execute_store_cpsr(u32 new_cpsr, u32 store_mask)
{
  reg[REG_CPSR] = (new_cpsr & store_mask) | (reg[REG_CPSR] & (~store_mask));
  extract_flags();
  if(store_mask & 0xFF)
  {
    set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0x1F]);
    // TODO: check for interrupts, since this can change PC it has to be
    // cased in ASM
  }
}
void function_cc execute_store_spsr(u32 new_spsr, u32 store_mask)
{
  u32 _spsr = spsr[reg[CPU_MODE]];
  spsr[reg[CPU_MODE]] = (new_spsr & store_mask) | (_spsr & (~store_mask));
}
u32 function_cc execute_load_s16(u32 address)
{
  u32 dest;
  read_memory_s16(address, dest);
  return dest;
}
u32 function_cc execute_aligned_load32(u32 address)
{
  u8 *map;
  if(!(address & 0xF0000000) && (map = memory_map_read[address >> 15]))
    return address32(map, address & 0x7FFF);
  else
    return read_memory32(address);
}
void function_cc execute_aligned_store32(u32 address, u32 source)
{
  u8 *map;

  if(!(address & 0xF0000000) && (map = memory_map_write[address >> 15]))
    address32(map, address & 0x7FFF) = source;
  else
    write_memory32(address, source);
}
u32 function_cc execute_lsl_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = value & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
      value <<= shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_lsr_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      if(shift == 32)
        reg[REG_C_FLAG] = (value >> 31) & 0x01;
      else
        reg[REG_C_FLAG] = 0;

      value = 0;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value >>= shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_asr_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    if(shift > 31)
    {
      value = (s32)value >> 31;
      reg[REG_C_FLAG] = value & 0x01;
    }
    else
    {
      reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
      value = (s32)value >> shift;
    }
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_ror_reg_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_lsl_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (32 - shift)) & 0x01;
    value <<= shift;
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_lsr_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    value >>= shift;
  }
  else
  {
    reg[REG_C_FLAG] = value >> 31;
    value = 0;
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_asr_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    value = (s32)value >> shift;
  }
  else
  {
    value = (s32)value >> 31;
    reg[REG_C_FLAG] = value & 0x01;
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_ror_imm_op(u32 value, u32 shift)
{
  if(shift != 0)
  {
    reg[REG_C_FLAG] = (value >> (shift - 1)) & 0x01;
    ror(value, value, shift);
  }
  else
  {
    u32 c_flag = reg[REG_C_FLAG];
    reg[REG_C_FLAG] = value & 0x01;
    value = (value >> 1) | (c_flag << 31);
  }

  calculate_flags_logic(value);
  return value;
}
u32 function_cc execute_neg(u32 rm)
{
  u32 dest = 0 - rm;
  calculate_flags_sub(dest, 0, rm);
  return dest;
}

u32 function_cc execute_load_u8(u32 address)
{
  u8 *map;
  u32 dest;

  if(((address & 0xF0000000) == 0) &&
   (map = memory_map_read[address >> 15]))
  {
    dest = map[address & 0x7FFF];
  }
  else
  {
    dest = read_memory8(address);
  }

  return dest;
}

u32 function_cc execute_load_s8(u32 address)
{
  return (s32)(s8)execute_load_u8(address);
}

u32 function_cc execute_load_u16(u32 address)
{
  u8 *map;
  u32 dest;

  if(((address & 0xF0000001) == 0) &&
   (map = memory_map_read[address >> 15]))
  {
    dest = *(u16 *)((u8 *)map + (address & 0x7FFF));
  }
  else
  {
    dest = read_memory16(address);
  }

  return dest;
}

u32 function_cc execute_load_u32(u32 address)
{
  u8 *map;
  u32 dest;

  if(((address & 0xF0000003) == 0) &&
   (map = memory_map_read[address >> 15]))
  {
    dest = *(u32 *)((u8 *)map + (address & 0x7FFF));
  }
  else
  {
    dest = read_memory32(address);
  }

  return dest;
}

u32 function_cc execute_mul_regs(u32 rm, u32 rs)
{
  u32 result = rm * rs;
  __asm__ __volatile__("mov.l %0,r4" : : "r" (result) : "r4");
  return result;
}

static void sh4_set_mul_result(u32 lo, u32 hi)
{
  __asm__ __volatile__(
    "mov.l %0,r4\n\t"
    "mov.l %1,r5"
    : : "r" (lo), "r" (hi) : "r4", "r5"
  );
}

void function_cc execute_mul_long_regs(u32 rm, u32 rs, u32 acc_lo, u32 acc_hi)
{
  u64 result = (u64)rm * (u64)rs + (((u64)acc_hi) << 32) + acc_lo;
  sh4_set_mul_result((u32)result, (u32)(result >> 32));
}

void function_cc execute_mul_long_s64(u32 rm, u32 rs)
{
  s64 result = (s64)(s32)rm * (s32)rs;
  sh4_set_mul_result((u32)result, (u32)((u64)result >> 32));
}

void function_cc execute_mul_long_u64(u32 rm, u32 rs)
{
  u64 result = (u64)rm * (u64)rs;
  sh4_set_mul_result((u32)result, (u32)(result >> 32));
}
void function_cc execute_swi(u32 pc)
{
  reg_mode[MODE_SUPERVISOR][6] = pc;
  collapse_flags();
  spsr[MODE_SUPERVISOR] = reg[REG_CPSR];
  reg[REG_CPSR] = (reg[REG_CPSR] & ~0x3F) | 0x13;
  set_cpu_mode(MODE_SUPERVISOR);
}
void swi_hle_div()
{
  s32 dividend = (s32)reg[0];
  s32 divisor = (s32)reg[1];
  s32 result = dividend / divisor;

  reg[0] = result;
  reg[1] = dividend % divisor;
  reg[3] = (result ^ (result >> 31)) - (result >> 31);
}
u8 swi_hle_handle[256] =
{
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0
};
