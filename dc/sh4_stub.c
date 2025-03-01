#include <kos.h>
#include "sh4_emit.h"

// External declarations
extern u32 reg[];
extern u32 spsr[];
extern u8* memory_map_read[];
extern u8* memory_map_write[];
extern u32 update_gba();
extern u8* block_lookup_address_arm(u32 pc);
extern u8* block_lookup_address_thumb(u32 pc);
extern u8* block_lookup_address_dual(u32 pc);
extern u8 read_memory8(u32 address);
extern u32 read_memory16(u32 address);
extern u32 read_memory32(u32 address);
extern cpu_alert_type write_memory8(u32 address, u8 value);
extern cpu_alert_type write_memory16(u32 address, u16 value);
extern cpu_alert_type write_memory32(u32 address, u32 value);
extern void flush_translation_cache_ram();
extern u32 step_debug(u32 pc, u32 cycles);

// Utility functions
static inline void collapse_flags() {
 reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) |
 (reg[REG_Z_FLAG] << 30) |
 (reg[REG_C_FLAG] << 29) |
 (reg[REG_V_FLAG ] << 28) |
 (reg[REG_CPSR] & 0xFF);
}

static inline void extract_flags() {
 reg[REG_N_FLAG] = (reg[REG_CPSR] >> 31) & 1;
 reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 1;
 reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 1;
 reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 1;
}

u32 sh4_update_gba(u32 pc) {
 u32 cycles;
 reg[REG_PC] = pc;
 collapse_flags();
 cycles = update_gba();
 if (reg[CHANGED_PC_STATUS] != 0) {
 u32 new_pc = reg[REG_PC];
 reg[CHANGED_PC_STATUS] = 0;
 if (reg[REG_CPSR] & 0x20) {
 return (u32)block_lookup_address_thumb(new_pc);
 } else {
 return (u32)block_lookup_address_arm(new_pc);
 }
 }
 return cycles;
}

void sh4_indirect_branch_arm(u32 address) {
 u8* target = block_lookup_address_arm(address);
 ((void (*)())target)();
}

void sh4_indirect_branch_thumb(u32 address) {
 u8* target = block_lookup_address_thumb(address);
 ((void (*)())target)();
}

void sh4_indirect_branch_dual(u32 address) {
 u8* target = block_lookup_address_dual(address);
 ((void (*)())target)();
}

void function_cc sh4_execute_store_u8(u32 address, u32 value) {
 reg[REG_PC] = address;
 if (!(address & 0xF0000000)) {
 u32 page = address >> 15;
 u8* map = memory_map_write[page];
 if (map) {
 u32 offset = address & 0x7FFF;
 map[offset] = (u8)value;
 if (map[offset - 32768] != 0) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 return;
 }
 }
 cpu_alert_type result = write_memory8(address, (u8)value);
 if (result != CPU_ALERT_NONE) {
 collapse_flags();
 if (result == CPU_ALERT_SMC) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 do {
 result = update_gba();
 } while (reg[CPU_HALT_STATE] != 0);
 goto lookup_pc;
 }
 return;

lookup_pc:
 reg[CHANGED_PC_STATUS] = 0;
 address = reg[REG_PC];
 if (reg[REG_CPSR] & 0x20) {
 ((void (*)())block_lookup_address_thumb(address))();
 } else {
 ((void (*)())block_lookup_address_arm(address))();
 }
}

void function_cc sh4_execute_store_u16(u32 address, u32 value) {
 address &= ~0x1;
 reg[REG_PC] = address;
 if (!(address & 0xF0000000)) {
 u32 page = address >> 15;
 u8* map = memory_map_write[page];
 if (map) {
 u32 offset = address & 0x7FFF;
 *(u16*)(map + offset) = (u16)value;
 if (*(u16*)(map + offset - 32768) != 0) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 return;
 }
 }
 cpu_alert_type result = write_memory16(address, (u16)value);
 if (result != CPU_ALERT_NONE) {
 collapse_flags();
 if (result == CPU_ALERT_SMC) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 do {
 result = update_gba();
 } while (reg[CPU_HALT_STATE] != 0);
 goto lookup_pc;
 }
 return;

lookup_pc:
 reg[CHANGED_PC_STATUS] = 0;
 address = reg[REG_PC];
 if (reg[REG_CPSR] & 0x20) {
 ((void (*)())block_lookup_address_thumb(address))();
 } else {
 ((void (*)())block_lookup_address_arm(address))();
 }
}

void function_cc sh4_execute_store_u32(u32 address, u32 value) {
 address &= ~0x3;
 reg[REG_PC] = address;
 if (!(address & 0xF0000000)) {
 u32 page = address >> 15;
 u8* map = memory_map_write[page];
 if (map) {
 u32 offset = address & 0x7FFF;
 *(u32*)(map + offset) = value;
 if (*(u32*)(map + offset - 32768) != 0) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 return;
 }
 }
 cpu_alert_type result = write_memory32(address, value);
 if (result != CPU_ALERT_NONE) {
 collapse_flags();
 if (result == CPU_ALERT_SMC) {
 flush_translation_cache_ram();
 goto lookup_pc;
 }
 do {
 result = update_gba();
 } while (reg[CPU_HALT_STATE] != 0);
 goto lookup_pc;
 }
 return;

lookup_pc:
 reg[CHANGED_PC_STATUS] = 0;
 address = reg[REG_PC];
 if (reg[REG_CPSR] & 0x20) {
 ((void (*)())block_lookup_address_thumb(address))();
 } else {
 ((void (*)())block_lookup_address_arm(address))();
 }
}

void translate_invalidate_dcache() {
    icache_flush_range((uint32)ram_translation_cache, 
                       (uint32)(ram_translation_ptr - ram_translation_cache) + 0x100);
}

void sh4_invalidate_icache_region(u32 addr, u32 size) {
    icache_flush_range(addr, size);
}

u32 function_cc sh4_execute_arm_translate(u32 cycles) {
    extract_flags();
    u32 pc = reg[REG_PC];
    u8* target = block_lookup_address_arm(pc);
    if (target != NULL) {
        printf("Dynarec ARM: Executing block at %p for PC %08x\n", target, pc);
        ((void (*)())target)();
    } else {
        printf("Dynarec ARM: Fallback at PC %08x\n", pc);
        // Add fallback interpreter call here if it exists
    }
    return cycles;
}
u32 function_cc sh4_execute_thumb_translate(u32 cycles) {
    extract_flags();
    u32 pc = reg[REG_PC];
    u8* target = block_lookup_address_thumb(pc);
    if (target != NULL) {
        printf("Dynarec Thumb: Executing block at %p for PC %08x\n", target, pc);
        ((void (*)())target)();
    } else {
        printf("Dynarec Thumb: Fallback at PC %08x\n", pc);
        // Add fallback interpreter call here if it exists
    }
    return cycles;
}


void sh4_step_debug(u32 pc) {
 collapse_flags();
 step_debug(pc, reg[REG_CYCLES]);
}