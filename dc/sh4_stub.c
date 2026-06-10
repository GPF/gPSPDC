#include <kos.h>
#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
#include <dc/video.h>
#include <dc/biosfont.h>
#endif
#include "sh4_emit.h"
#include "cheats.h"

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
extern void gpsp_dynarec_fatal_error(const char *detail);

#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
static u32 sh4_trace_y = 396;
static u32 sh4_trace_last_swi = 0xFFFFFFFF;
static u32 sh4_trace_last_swi_pc = 0;
static u32 sh4_trace_last_swi_thumb = 0;
static u32 sh4_trace_last_emit_pc = 0xFFFFFFFF;
static u32 sh4_trace_last_emit_source_pc = 0xFFFFFFFF;
static u32 sh4_trace_last_emit_opcode = 0;
static u32 sh4_trace_emit_count = 0;

static void sh4_trace_screen(const char *message)
{
  u32 y;

  for(y = 0; y < 18; y++)
    memset(vram_s + ((sh4_trace_y + y) * 640) + 24, 0, 592 * 2);

  bfont_draw_str(vram_s + (sh4_trace_y * 640) + 24, 640, 0,
   (char *)message);
  sh4_trace_y += 20;
  if(sh4_trace_y > 456)
    sh4_trace_y = 396;
}

static void sh4_trace_block_words(u32 pc, u8 *target, u32 cycles)
{
  char buffer[96];
  u16 *words = (u16 *)target;
  u16 *dest;
  u32 y;

  for(y = 324; y < 374; y++)
    memset(vram_s + (y * 640) + 24, 0, 592 * 2);

  sprintf(buffer, "dyn pc=%08x tgt=%08x cyc=%u", pc, (u32)target, cycles);
  dest = vram_s + (324 * 640) + 24;
  bfont_draw_str(dest, 640, 0, buffer);

  if(target != NULL)
  {
    sprintf(buffer, "%04x %04x %04x %04x %04x %04x %04x %04x",
     words[0], words[1], words[2], words[3],
     words[4], words[5], words[6], words[7]);
    dest = vram_s + (348 * 640) + 24;
    bfont_draw_str(dest, 640, 0, buffer);
  }
}
#endif

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

/* Stack base captured inside execute_arm_translate.  Translated code and
   every re-entry into it runs with r15 reset to this base: blocks never
   return, so any C frame still live when control dives into a new block
   would otherwise leak permanently (the x86 stub pops its return address
   in lookup_pc for the same reason).  Without the reset, every IRQ
   dispatch, halt wake-up, and SMC flush grows the stack until it
   overflows mid-game. */
static u32 sh4_dispatch_stack;

static void __attribute__((noreturn, noinline))
 sh4_dispatch_block(u8 *target, u32 cycles)
{
  __asm__ __volatile__(
    "mov %[stk], r15\n\t"
    "mov %[regptr], r12\n\t"
    "mov %[cyc], r13\n\t"
    "jmp @%[tgt]\n\t"
    "nop\n\t"
    :
    : [tgt] "r" (target), [regptr] "r" (reg), [cyc] "r" (cycles),
      [stk] "r" (sh4_dispatch_stack)
    : "memory"
  );
  __builtin_unreachable();
}

static void __attribute__((noreturn)) sh4_lookup_pc(u32 cycles)
{
  u32 pc = reg[REG_PC];
  u8 *target;

  reg[CHANGED_PC_STATUS] = 0;

  if(reg[REG_CPSR] & 0x20)
    target = block_lookup_address_thumb(pc);
  else
    target = block_lookup_address_arm(pc);

  sh4_dispatch_block(target, cycles);
}

u32 sh4_update_gba(u32 pc)
{
  static u32 trace_update_count;
  u32 cycles;

  reg[REG_PC] = pc;
  collapse_flags();
  cycles = update_gba();
#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
  trace_update_count++;
  if((trace_update_count & 63) == 1)
  {
    char buffer[96];
    sprintf(buffer, "u%u p=%08x e=%08x @%08x",
     trace_update_count, pc, sh4_trace_last_emit_pc,
     sh4_trace_last_emit_source_pc);
    sh4_trace_screen(buffer);
    sprintf(buffer, "op=%08x ec=%u s=%02x r=%08x",
     sh4_trace_last_emit_opcode, sh4_trace_emit_count,
     sh4_trace_last_swi == 0xFFFFFFFF ? 0xFF : sh4_trace_last_swi,
     sh4_trace_last_swi_pc);
    sh4_trace_screen(buffer);
  }
#endif

  if(reg[CHANGED_PC_STATUS] != 0)
    sh4_lookup_pc(cycles);

  return cycles;
}

void sh4_indirect_branch_arm(u32 address, u32 cycles)
{
  sh4_dispatch_block(block_lookup_address_arm(address), cycles);
}

void sh4_indirect_branch_thumb(u32 address, u32 cycles)
{
  sh4_dispatch_block(block_lookup_address_thumb(address), cycles);
}

void sh4_indirect_branch_dual(u32 address, u32 cycles)
{
  sh4_dispatch_block(block_lookup_address_dual(address), cycles);
}

void function_cc execute_store_u8(u32 address, u32 value, u32 pc, u32 cycles)
{
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
        sh4_lookup_pc(cycles);
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
        sh4_lookup_pc(cycles);
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_lookup_pc(cycles);
    }
  }

  return;
}

void function_cc execute_store_u16(u32 address, u32 value, u32 pc, u32 cycles)
{
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
        sh4_lookup_pc(cycles);
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
        sh4_lookup_pc(cycles);
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_lookup_pc(cycles);
    }
  }

  return;
}

void function_cc execute_store_u32(u32 address, u32 value, u32 pc, u32 cycles)
{
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
        sh4_lookup_pc(cycles);
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
        sh4_lookup_pc(cycles);
      }

      do
      {
        cycles = update_gba();
      }
      while(reg[CPU_HALT_STATE] != 0);

      sh4_lookup_pc(cycles);
    }
  }

  return;
}

void sh4_invalidate_icache_region(u32 addr, u32 size)
{
  /* Freshly emitted code is written through the operand cache; it must be
     written back to RAM before the instruction cache refetches the range. */
  dcache_flush_range(addr, size);
  icache_flush_range(addr, size);
}

u32 function_cc execute_arm_translate(u32 cycles)
{
  u8 *target;
  u32 pc = reg[REG_PC];

  extract_flags_local();
  target = block_lookup_address_arm(pc);

#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
  printf("[gbaDC trace] execute dynarec enter pc=%08x target=%p cycles=%u cpsr=%08x\n",
   pc, target, cycles, reg[REG_CPSR]);
  sh4_trace_block_words(pc, target, cycles);
#endif

  if(target == NULL)
  {
    char buffer[64];
    sprintf(buffer, "null translation at %08x", pc);
    gpsp_dynarec_fatal_error(buffer);
    return cycles;
  }

  /* Record the stack base every block (re-)entry resets to; translated
     code never returns here. */
  __asm__ __volatile__("mov r15, %0" : "=r" (sh4_dispatch_stack));
  sh4_dispatch_block(target, cycles);
}

void sh4_step_debug(u32 pc, u32 cycles)
{
  collapse_flags();
  step_debug(pc, cycles);
}

void sh4_trace_swi(u32 swi_number, u32 pc, u32 thumb)
{
#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
  sh4_trace_last_swi = swi_number;
  sh4_trace_last_swi_pc = pc;
  sh4_trace_last_swi_thumb = thumb;
#else
  (void)swi_number;
  (void)pc;
  (void)thumb;
#endif
}

void sh4_trace_emit_update_pc(u32 new_pc, u32 source_pc, u32 opcode)
{
#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
  sh4_trace_last_emit_pc = new_pc;
  sh4_trace_last_emit_source_pc = source_pc;
  sh4_trace_last_emit_opcode = opcode;
  sh4_trace_emit_count++;
#else
  (void)new_pc;
  (void)source_pc;
  (void)opcode;
#endif
}

void sh4_cheat_hook(void)
{
  process_cheats();
}
