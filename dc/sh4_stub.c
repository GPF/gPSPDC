#include <kos.h>
#include "sh4_emit.h"

extern u32 reg[];
extern u32 spsr[];
extern u8 *memory_map_read[];
extern u8 *memory_map_write[];
extern u32 update_gba();
extern u8 *block_lookup_address_arm(u32 pc);
extern u8 *block_lookup_address_thumb(u32 pc);
extern u8 *block_lookup_address_dual(u32 pc);
extern u8 read_memory8(u32 address);
extern u32 read_memory16(u32 address);
extern u32 read_memory32(u32 address);
extern cpu_alert_type write_memory8(u32 address, u8 value);
extern cpu_alert_type write_memory16(u32 address, u16 value);
extern cpu_alert_type write_memory32(u32 address, u32 value);
extern void flush_translation_cache_ram();
extern u32 step_debug(u32 pc, u32 cycles);

static inline void collapse_flags(void)
{
  reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) |
   (reg[REG_Z_FLAG] << 30) |
   (reg[REG_C_FLAG] << 29) |
   (reg[REG_V_FLAG] << 28) |
   (reg[REG_CPSR] & 0xFF);
}

static inline void extract_flags_local(void)
{
  reg[REG_N_FLAG] = (reg[REG_CPSR] >> 31) & 1;
  reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 1;
  reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 1;
  reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 1;
}

static inline void sh4_reload_cycles(u32 cycles)
{
  __asm__ __volatile__("mov.l %0, r13" : : "r" (cycles) : "r13", "memory");
}

static void sh4_lookup_pc(void)
{
  u32 pc = reg[REG_PC];

  reg[CHANGED_PC_STATUS] = 0;

  if(reg[REG_CPSR] & 0x20)
    ((void (*)(void))block_lookup_address_thumb(pc))();
  else
    ((void (*)(void))block_lookup_address_arm(pc))();
}

u32 sh4_update_gba(u32 pc)
{
  u32 cycles;
  u32 new_pc;

  reg[REG_PC] = pc;
  collapse_flags();
  cycles = update_gba();

  if(reg[CHANGED_PC_STATUS] != 0)
  {
    new_pc = reg[REG_PC];
    reg[CHANGED_PC_STATUS] = 0;
    sh4_reload_cycles(cycles);

    if(reg[REG_CPSR] & 0x20)
      ((void (*)(void))block_lookup_address_thumb(new_pc))();
    else
      ((void (*)(void))block_lookup_address_arm(new_pc))();
  }

  return cycles;
}

void sh4_indirect_branch_arm(u32 address)
{
  u8 *target = block_lookup_address_arm(address);
  ((void (*)(void))target)();
}

void sh4_indirect_branch_thumb(u32 address)
{
  u8 *target = block_lookup_address_thumb(address);
  ((void (*)(void))target)();
}

void sh4_indirect_branch_dual(u32 address)
{
  u8 *target = block_lookup_address_dual(address);
  ((void (*)(void))target)();
}

void function_cc execute_store_u8(u32 address, u32 value, u32 pc)
{
  u32 cycles;

  reg[REG_PC] = pc;

  if(!(address & 0xF0000000))
  {
    u32 page = address >> 15;
    u8 *map = memory_map_write[page];

    if(map)
    {
      u32 offset = address & 0x7FFF;
      map[offset] = (u8)value;

      if(map[offset - 32768] != 0)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
      }

      return;
    }
  }

  {
    cpu_alert_type result = write_memory8(address, (u8)value);

    if(result != CPU_ALERT_NONE)
    {
      collapse_flags();

      if(result == CPU_ALERT_SMC)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
        return;
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_reload_cycles(cycles);
      sh4_lookup_pc();
    }
  }

  return;
}

void function_cc execute_store_u16(u32 address, u32 value, u32 pc)
{
  u32 cycles;

  address &= ~0x1;
  reg[REG_PC] = pc;

  if(!(address & 0xF0000000))
  {
    u32 page = address >> 15;
    u8 *map = memory_map_write[page];

    if(map)
    {
      u32 offset = address & 0x7FFF;
      *(u16 *)(map + offset) = (u16)value;

      if(*(u16 *)(map + offset - 32768) != 0)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
      }

      return;
    }
  }

  {
    cpu_alert_type result = write_memory16(address, (u16)value);

    if(result != CPU_ALERT_NONE)
    {
      collapse_flags();

      if(result == CPU_ALERT_SMC)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
        return;
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_reload_cycles(cycles);
      sh4_lookup_pc();
    }
  }

  return;
}

void function_cc execute_store_u32(u32 address, u32 value, u32 pc)
{
  u32 cycles;

  address &= ~0x3;
  reg[REG_PC] = pc;

  if(!(address & 0xF0000000))
  {
    u32 page = address >> 15;
    u8 *map = memory_map_write[page];

    if(map)
    {
      u32 offset = address & 0x7FFF;
      *(u32 *)(map + offset) = value;

      if(*(u32 *)(map + offset - 32768) != 0)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
      }

      return;
    }
  }

  {
    cpu_alert_type result = write_memory32(address, value);

    if(result != CPU_ALERT_NONE)
    {
      collapse_flags();

      if(result == CPU_ALERT_SMC)
      {
        flush_translation_cache_ram();
        sh4_lookup_pc();
        return;
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_reload_cycles(cycles);
      sh4_lookup_pc();
    }
  }

  return;
}

void sh4_invalidate_icache_region(u32 addr, u32 size)
{
  icache_flush_range(addr, size);
}

u32 function_cc execute_arm_translate(u32 cycles)
{
  u8 *target;
  u32 pc = reg[REG_PC];

  extract_flags_local();
  target = block_lookup_address_arm(pc);

  __asm__ __volatile__(
    "mov.l %[regptr], r12\n\t"
    "mov.l %[cyc], r13\n\t"
    "jmp @%[tgt]\n"
    :
    : [tgt] "r" (target), [regptr] "r" (reg), [cyc] "r" (cycles)
    : "r12", "r13", "memory"
  );

  return cycles;
}

void sh4_step_debug(u32 pc)
{
  collapse_flags();
  step_debug(pc, reg[REG_CYCLES]);
}
