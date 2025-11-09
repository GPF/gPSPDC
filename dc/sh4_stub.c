// sh4_stub.c
// #include "sh4_emit.h"
#include <kos.h>
#ifndef DEBUG_DYNAREC
#define DEBUG_DYNAREC 0 // Default to off; enable with -DDEBUG_DYNAREC=1 in Makefile
#endif

#if DEBUG_DYNAREC
#define SH4_LOG(level, fmt, ...) dbglog(level, fmt, ##__VA_ARGS__)
#else
#define SH4_LOG(level, fmt, ...) do { } while (0) // No-op in release
#endif


typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long int u64;
typedef signed long long int s64;
register volatile u32* sh4_reg_base asm("r12");
register volatile u32 sh4_reg_cycles asm("r13");
#define function_cc
#define block_prologue_size 8
extern u32 reg[64];
extern  u32 spsr[6];
extern u32 cpu_modes[32];
extern u16 palette_ram[512];
extern u16 oam_ram[512];
extern u16 palette_ram_converted[512];
extern u16 io_registers[1024 * 16];
extern u8 ewram[1024 * 256 * 2];
extern u8 iwram[1024 * 32 * 2];
extern u8 vram[1024 * 96 * 2];
extern u32 reg_mode[7][7];
extern u8 *memory_map_read[8 * 1024];
extern u8 *memory_map_write[8 * 1024];

#define address32(base, offset) *((u32 *)((u8 *)base + (offset)))
#define ROM_BRANCH_HASH_SIZE (1024 * 64)
extern u32 iwram_code_min;
extern u32 iwram_code_max;
extern u32 ewram_code_min;
extern u32 ewram_code_max;

extern u32 *rom_branch_hash[ROM_BRANCH_HASH_SIZE];

extern u8 swi_hle_handle[256];

#define REG_SP            (13)
#define REG_LR            (14)
#define REG_PC            (15)
#define REG_N_FLAG        (16)
#define REG_Z_FLAG        (17)
#define REG_C_FLAG        (18)
#define REG_V_FLAG        (19)
#define REG_CPSR          (20)
#define REG_SAVE          (21)
#define REG_SAVE2         (22)
#define REG_SAVE3         (23)
#define CPU_MODE          (29)
#define CPU_HALT_STATE    (30)
#define CHANGED_PC_STATUS (31)

#define REG_IE   (0x100)
#define REG_IF   (0x101)
#define REG_IME  (0x104)

#define MODE_USER        0
#define MODE_IRQ         1
#define MODE_FIQ         2
#define MODE_SUPERVISOR  3
#define MODE_ABORT       4
#define MODE_UNDEFINED   5
#define MODE_INVALID     6

#define NULL ((void *)0)
extern u32 oam_update;
extern u16 palette_ram[];    // 16-bit aligned palette RAM
extern u8  vram[];           // raw byte-addressable VRAM
extern u16  oam_ram[];        // OAM memory

// Optional if you access I/O as an array
extern u16  io_registers[];
void function_cc ext_store_u8(u32 address, u32 value, u32 pc);
void function_cc ext_store_u16(u32 address, u32 value, u32 pc);
void function_cc ext_store_u32(u32 address, u32 value, u32 pc);

// Handlers
void function_cc ext_store_ignore(u32 address, u32 value, u32 pc);

void function_cc ext_store_io8(u32 address, u32 value, u32 pc);
void function_cc ext_store_io16(u32 address, u32 value, u32 pc);
void function_cc ext_store_io32(u32 address, u32 value, u32 pc);

void function_cc ext_store_palette8(u32 address, u32 value, u32 pc);
void function_cc ext_store_palette16(u32 address, u32 value, u32 pc);
void function_cc ext_store_palette16b(u32 address, u16 value, u32 pc);
void function_cc ext_store_palette32(u32 address, u32 value, u32 pc);

void function_cc ext_store_vram8(u32 address, u32 value, u32 pc);
void function_cc ext_store_vram16(u32 address, u32 value, u32 pc);
void function_cc ext_store_vram32(u32 address, u32 value, u32 pc);

void function_cc ext_store_oam8(u32 address, u32 value, u32 pc);
void function_cc ext_store_oam16(u32 address, u32 value, u32 pc);
void function_cc ext_store_oam32(u32 address, u32 value, u32 pc);

void function_cc ext_store_eeprom(u32 address, u32 value, u32 pc);
void function_cc ext_store_rtc(u32 address, u32 value, u32 pc);
extern u32 update_gba(u32 cycles);
extern u8* block_lookup_address_arm(u32 pc);
extern u8* block_lookup_address_thumb(u32 pc);
extern u8* block_lookup_address_dual(u32 pc);

// u8* current_block_start = NULL; // Pointer to the start of the current block
// u8* branch_patch_ptr = NULL;



void sync_reg_from_r12() {
    u32* live_ptr;
    asm volatile ("mov r12, %0" : "=r"(live_ptr));
    memcpy(reg, live_ptr, sizeof(u32) * 64);
}

static inline u32 get_sh4_pr() {
    u32 pr;
    asm volatile ("sts pr, %0" : "=r" (pr));
    return pr;
}

static inline u32 get_sh4_r15() {
    u32 r15;
    asm volatile ("mov r15, %0" : "=r" (r15));
    return r15;
}

void log_block_start() {
    sh4_reg_base=reg;
    u32 r13_direct;
    asm volatile ("mov r13, %0" : "=r"(r13_direct));
    // sync_reg_from_r12();
    // log_sh4_registers("log_block_start");
    // u32 saved_cycles;
    // asm volatile ("mov.l @(4, r15), %0" : "=r"(saved_cycles)); // Read saved r13 from stack

    printf("r13 direct: %u\n", r13_direct);
    u32 pc = sh4_reg_base[15];  // REG_PC
    u8* block_addr = NULL;
    u32 hash_target = ((pc * 2654435761U) >> 16) & (ROM_BRANCH_HASH_SIZE - 1);
    u32* block_ptr = rom_branch_hash[hash_target];
    while (block_ptr) {
        if (block_ptr[0] == pc) {
            block_addr = (u8*)(block_ptr + 2);
            break;
        }
        block_ptr = (u32*)block_ptr[1];
    }
    // log_sh4_registers("log_block_start");
    printf("▶️ Block Start: REG_PC=%08x, cycles=%u, block_addr=%p, pr=%08x\n",
           pc, r13_direct, block_addr, get_sh4_pr());
           dump_block(block_addr, 256);
}

void log_block_exit() {
    u32 saved_cycles;
    asm volatile ("mov r13, %0" : "=r"(saved_cycles));
    u32 pc = sh4_reg_base[15];
    u8* block_addr = NULL;
    u32 hash_target = ((pc * 2654435761U) >> 16) & (ROM_BRANCH_HASH_SIZE - 1);
    u32* block_ptr = rom_branch_hash[hash_target];
    while (block_ptr) {
        if (block_ptr[0] == pc) {
            block_addr = (u8*)(block_ptr + 2);
            break;
        }
        block_ptr = (u32*)block_ptr[1];
    }
    printf("⏹️ Block Exit: REG_PC=%08x, cycles=%d, block_addr=%p, pr=%08x\n",
           pc, saved_cycles, block_addr, get_sh4_pr());
}

void log_arm_state(const char* context) {
    // sh4_reg_base=reg;
    u32 pc = sh4_reg_base[REG_PC];          // ARM R15 (Program Counter)
    u32 lr = sh4_reg_base[REG_LR];          // ARM R14 (Link Register)
    u32 r0 = sh4_reg_base[0];               // ARM R0
    u32 r1 = sh4_reg_base[1];               // ARM R1
    u32 r2 = sh4_reg_base[2];               // ARM R2
    u32 r3 = sh4_reg_base[3];               // ARM R3
    u32 r4 = sh4_reg_base[4];               // ARM R4
    u32 r5 = sh4_reg_base[5];               // ARM R5
    u32 n_flag = sh4_reg_base[REG_N_FLAG];  // ARM Negative Flag
    u32 z_flag = sh4_reg_base[REG_Z_FLAG];  // ARM Zero Flag
    u32 c_flag = sh4_reg_base[REG_C_FLAG];  // ARM Carry Flag
    u32 v_flag = sh4_reg_base[REG_V_FLAG];  // ARM Overflow Flag
    u32 halt_state = sh4_reg_base[CPU_HALT_STATE];  // Emulator Halt State
    u32 cycles = sh4_reg_cycles;            // Cycle count from SH-4 R13
    u32 changepc = sh4_reg_base[CHANGED_PC_STATUS];            // Cycle count from SH-4 R13

    printf("%s: ARM_PC=%08x, ARM_LR=%08x, ARM_R0=%08x, ARM_R1=%08x, ARM_R2=%08x, ARM_R3=%08x, ARM_R4=%08x, ARM_R5=%08x, sh4_reg_base[CHANGED_PC_STATUS]=%d\n",
           context, pc, lr, r0, r1, r2, r3, r4, r5, changepc);
    printf("ARM Flags: N=%d, Z=%d, C=%d, V=%d, Cycles=%u, HALT=%d\n",
           n_flag, z_flag, c_flag, v_flag, cycles, halt_state);
}

void log_sh4_registers(const char *context) {
    u32 r[16] __attribute__((aligned(4)));
    u32 pr;

    // Capture all SH-4 general-purpose registers (R0–R15) and PR
    asm volatile (
        "mov.l r0, @(0,%1)\n\t"
        "mov.l r1, @(4,%1)\n\t"
        "mov.l r2, @(8,%1)\n\t"
        "mov.l r3, @(12,%1)\n\t"
        "mov.l r4, @(16,%1)\n\t"
        "mov.l r5, @(20,%1)\n\t"
        "mov.l r6, @(24,%1)\n\t"
        "mov.l r7, @(28,%1)\n\t"
        "mov.l r8, @(32,%1)\n\t"
        "mov.l r9, @(36,%1)\n\t"
        "mov.l r10, @(40,%1)\n\t"
        "mov.l r11, @(44,%1)\n\t"
        "mov.l r12, @(48,%1)\n\t"
        "mov.l r13, @(52,%1)\n\t"
        "mov.l r14, @(56,%1)\n\t"
        "mov.l r15, @(60,%1)\n\t"
        "sts pr, %0\n\t"
        : "=r"(pr) // Output: PR
        : "r"(r)   // Input: pointer to r[] array
        : "memory" // Clobber memory
    );

    // Log all registers with context
    printf( "[%s] SH-4 Registers:\n", context);
    printf("  R0=%08x  R1=%08x  R2=%08x  R3=%08x\n", r[0], r[1], r[2], r[3]);
    printf("  R4=%08x  R5=%08x  R6=%08x  R7=%08x\n", r[4], r[5], r[6], r[7]);
    printf( "  R8=%08x  R9=%08x R10=%08x R11=%08x\n", r[8], r[9], r[10], r[11]);
    printf( " R12=%08x R13=%08x R14=%08x R15=%08x\n", r[12], r[13], r[14], r[15]);
    printf( "  PR=%08x\n", pr);
}

void log_sh4_registers_wrapper() {
    u32 r15_now = get_sh4_r15();
    SH4_LOG(DBG_INFO, "[runtime-check] SP(R15) = %08x\n", r15_now);
    log_sh4_registers("runtime-check");
}

void log_indirect_branch(u32 target, u32 block_start) {
    SH4_LOG(DBG_INFO, "log Indirect branch from block %p to target %08x\n", block_start, target);
}

void dump_block(u8* block_address, size_t size) {
    printf("Block dump at %p:\n", block_address);
    for (size_t i = 0; i < size; i += 2) {
        if (i % 16 == 0) printf("\n%08x: ", (u32)(block_address + i));
        printf("%04x ", *(u16*)(block_address + i));
    }
    printf("\n");
}



u32 execute_lookup_pc(u32 cycles) {
    sh4_reg_base = reg;
    u32 pc= sh4_reg_base[REG_PC];
    sh4_reg_cycles = cycles;
    SH4_LOG(DBG_DEBUG, "execute_lookup_pc: PC=%08x, cycles=%d\n", pc, cycles);
    
    if (pc & 0xF0000000) {
        SH4_LOG(DBG_ERROR, "Invalid PC %08x\n", pc);
        sh4_reg_base[CPU_HALT_STATE] = 1;
        return cycles;
    }
    
    u32 cpsr = sh4_reg_base[REG_CPSR];
    u8* target = (cpsr & 0x20) ? block_lookup_address_thumb(pc)
                               : block_lookup_address_arm(pc);
    target -= block_prologue_size;
    
    if ((u32)target < 0x8c000000 || (u32)target > 0x8d000000 || !target || ((u32)target & 0x3)) {
        SH4_LOG(DBG_ERROR, "Invalid block target %p for PC=%08x\n", target, pc);
        sh4_reg_base[CPU_HALT_STATE] = 1;
        return cycles;
    }
    // log_sh4_registers("execute_lookup_pc");
    log_arm_state("execute_lookup_pc");
    SH4_LOG(DBG_INFO, "execute_lookup_pc: Jumping to block %p for PC=%08x\n", target, pc);
    asm volatile(
        "jmp @%0\n\t"
        "nop\n\t"
        : /* no outputs */
        : "r"(target)
        : "pr", "memory"
    );
    
    return cycles;
}

u32 sh4_update_gba(u32 pc) {
    SH4_LOG(DBG_INFO, "sh4_update_gba: Entry with PC=%08x (R4)\n", pc);
    // log_arm_state("sh4_update_gba");
    sh4_reg_base = reg; // reg is the pointer to the ARM register array
    sh4_reg_base[REG_PC] = pc;
    collapse_flags();
    u32 old_cycles = sh4_reg_cycles;
    SH4_LOG(DBG_INFO, "sh4_update_gba: Exit with old_cycles=%u\n", old_cycles);
    sh4_reg_cycles = update_gba(old_cycles);
    
    SH4_LOG(DBG_INFO, "sh4_update_gba: Exit with new_cycles=%u\n", sh4_reg_cycles);
    
    u32 changed_pc = sh4_reg_base[CHANGED_PC_STATUS];
    // if (sh4_reg_base != reg) {
    //     printf("⚠️ sh4_reg_base != reg -- reg[CHANGED_PC_STATUS]=%u, sh4_reg_base[CHANGED_PC_STATUS]=%u\n",
    //            reg[CHANGED_PC_STATUS], sh4_reg_base[CHANGED_PC_STATUS]);
    // }
    if (changed_pc == 1) {
        SH4_LOG(DBG_DEBUG, "✅ changed_pc IS SET: %08x, REG_PC=%08x\n", changed_pc, sh4_reg_base[REG_PC]);
        sh4_reg_base[CHANGED_PC_STATUS] = 0;
        sh4_reg_cycles = execute_lookup_pc(sh4_reg_cycles);
        return sh4_reg_cycles | 0x80000000;
    } else {
        SH4_LOG(DBG_ERROR, "🚨 changed_pc NOT set! REG_PC=%08x\n", sh4_reg_base[REG_PC]);
    }
    SH4_LOG(DBG_INFO, "sh4_update_gba: Exit with cycles=%u\n", sh4_reg_cycles);
    return sh4_reg_cycles;
}

u32 execute_arm_translate(u32 cycles) {
    // Initialize registers
    sh4_reg_base = reg;
    sh4_reg_cycles = cycles;
    log_sh4_registers("execute_arm_translate");
    extract_flags();
    u32 pc = sh4_reg_base[REG_PC];  // ARM R15 (Program Counter)
    u32 cpsr = sh4_reg_base[REG_CPSR];  // ARM CPSR (Current Program Status Register)
    printf("REG_PC = %08x, REG_CPSR = %08x -> %s path\n",
        pc, cpsr, (cpsr & 0x20) ? "Thumb" : "ARM");
    u8* block_address = (cpsr & 0x20) ? block_lookup_address_thumb(pc) 
                                      : block_lookup_address_arm(pc);
    block_address -= block_prologue_size;

    if ((u32)block_address < 0x8c000000 || (u32)block_address > 0x8d000000) {
        SH4_LOG(DBG_ERROR, "Invalid block address %p for PC=%08x\n", block_address, pc);
        sh4_reg_base[CPU_HALT_STATE] = 1;  // Emulator halt state
        return sh4_reg_cycles;
    }

    log_sh4_registers("execute_arm_translate");
    // Save r13 before calling dump_block
    u32 saved_r13;
    asm volatile ("mov r13, %0" : "=r"(saved_r13));   
    printf("r13 before dump_block: %u\n", saved_r13);
    // dump_block(block_address, 256);
    // asm volatile ("mov %0, r13, " : : "r"(saved_r13));
    printf("execute_arm_translate: Jumping to block at %p, PC=%08x\n", block_address, pc);
    // Restore r13 after dump_block returns

    // Call the block using inline assembly
    u32 r13_before, r13_after;
    asm volatile ("mov r13, %0" : "=r"(r13_before));
    printf("r13 before: %u\n", r13_before);
    asm volatile ("jsr @%0\n\t nop\n\t" : : "r"(block_address));
    log_sh4_registers("After block exit");
    asm volatile ("mov r13, %0" : "=r"(r13_after));
    printf("r13 before: %u, after: %u\n", r13_before, r13_after);

    return sh4_reg_cycles;
}



typedef void (*ext_store_handler)(u32 address, u32 value, u32 pc);

static const ext_store_handler ext_store_u8_jtable[16] = {
    ext_store_ignore,  // 0x00 BIOS
    ext_store_ignore,  // 0x01 invalid
    ext_store_ignore,  // 0x02 EWRAM (already handled)
    ext_store_ignore,  // 0x03 IWRAM (already handled)
    ext_store_io8,     // 0x04 I/O
    ext_store_palette8,// 0x05 Palette RAM
    ext_store_vram8,   // 0x06 VRAM
    ext_store_oam8,    // 0x07 OAM
    ext_store_ignore,  // 0x08 gamepak
    ext_store_ignore,  // 0x09 gamepak
    ext_store_ignore,  // 0x0A gamepak
    ext_store_ignore,  // 0x0B gamepak
    ext_store_ignore,  // 0x0C gamepak
    ext_store_eeprom,  // 0x0D EEPROM
    ext_store_ignore,  // 0x0E Flash/SRAM
    ext_store_ignore   // 0x0F unused
};

static const ext_store_handler ext_store_u16_jtable[16] = {
    ext_store_ignore,      // 0x00 BIOS
    ext_store_ignore,      // 0x01 Invalid
    ext_store_ignore,      // 0x02 EWRAM (handled earlier)
    ext_store_ignore,      // 0x03 IWRAM (handled earlier)
    ext_store_io16,        // 0x04 I/O
    ext_store_palette16,   // 0x05 Palette RAM
    ext_store_vram16,      // 0x06 VRAM
    ext_store_oam16,       // 0x07 OAM
    ext_store_rtc,         // 0x08 Gamepak/RTC
    ext_store_ignore,      // 0x09
    ext_store_ignore,      // 0x0A
    ext_store_ignore,      // 0x0B
    ext_store_ignore,      // 0x0C
    ext_store_eeprom,      // 0x0D EEPROM
    ext_store_ignore,      // 0x0E (Flash/SRAM is 8-bit only)
    ext_store_ignore       // 0x0F
};

static const ext_store_handler ext_store_u32_jtable[16] = {
    ext_store_ignore,      // 0x00 BIOS
    ext_store_ignore,      // 0x01 Invalid
    ext_store_ignore,      // 0x02 EWRAM
    ext_store_ignore,      // 0x03 IWRAM
    ext_store_io32,        // 0x04 I/O
    ext_store_palette32,   // 0x05 Palette RAM
    ext_store_vram32,      // 0x06 VRAM
    ext_store_oam32,       // 0x07 OAM
    ext_store_ignore,      // 0x08 Gamepak (no RTC in 32-bit)
    ext_store_ignore,      // 0x09
    ext_store_ignore,      // 0x0A
    ext_store_ignore,      // 0x0B
    ext_store_ignore,      // 0x0C
    ext_store_eeprom,      // 0x0D EEPROM
    ext_store_ignore,      // 0x0E (Flash/SRAM is 8-bit only)
    ext_store_ignore       // 0x0F
};

void function_cc ext_store_rtc(u32 address, u32 value, u32 pc) {
    // GBA RTC often mapped around 0x080000C4, or handled via MMIO-like behavior
    write_rtc(address, (u16)value);  // Replace with real RTC logic if needed
}

void function_cc ext_store_u8(u32 address, u32 value, u32 pc) {
    u32 region = address >> 24;
    if (region > 15) {
        ext_store_ignore(address, value, pc);
        return;
    }
    ext_store_u8_jtable[region](address, value, pc);
}


void function_cc ext_store_u16(u32 address, u32 value, u32 pc) {
    u32 region = address >> 24;
    if (region > 15) {
        ext_store_ignore(address, value, pc);
        return;
    }
    ext_store_u16_jtable[region](address, value, pc);
}

void function_cc ext_store_u32(u32 address, u32 value, u32 pc) {
    u32 region = address >> 24;
    if (region > 15) {
        ext_store_ignore(address, value, pc);
        return;
    }
    ext_store_u32_jtable[region](address, value, pc);
}

void function_cc ext_store_ignore(u32 address, u32 value, u32 pc) {
    // Ignored write, typically BIOS or unmapped region
    (void)address;
    (void)value;
    (void)pc;
}

void function_cc write_epilogue(u32 result) {
    if (result == 0) return;

    collapse_flags();

    if (result == 2) { // CPU_ALERT_SMC
        execute_lookup_pc(sh4_reg_cycles);
        return;
    }

    do {
        result = update_gba(sh4_reg_cycles);
    } while (sh4_reg_base[CPU_HALT_STATE] != 0);

    execute_lookup_pc(sh4_reg_cycles);
}

void function_cc ext_store_io8(u32 address, u32 value, u32 pc) {
    address &= 0x3FF;
    u32 result = write_io_register8(address, (u8)value);
    if (result != 0) {
        write_epilogue(result);
    }
}

void function_cc ext_store_io16(u32 address, u32 value, u32 pc) {
    address &= 0x3FF;
    u32 result = write_io_register16(address, (u16)value);
    if (result != 0) {
        write_epilogue(result);
    }
}

void function_cc ext_store_io32(u32 address, u32 value, u32 pc) {
    address &= 0x3FF; // wrap to 0x000–0x3FF
    u32 result = write_io_register32(address, value);
    if (result != 0) {
        write_epilogue(result);
    }
}

void function_cc ext_store_palette8(u32 address, u32 value, u32 pc) {
    address &= 0x3FF;
    ((u8 *)palette_ram)[address] = (u8)value;
}

void function_cc ext_store_palette16(u32 address, u32 value, u32 pc) {
    address &= 0x3FF;
    palette_ram[address >> 1] = (u16)value;
}

void function_cc ext_store_palette16b(u32 address, u16 value, u32 pc) {
    // Example implementation:
    palette_ram[address >> 1] = value;
}

void function_cc ext_store_palette32(u32 address, u32 value, u32 pc) {
    address &= 0x3FF;

    u16 lower = (u16)(value & 0xFFFF);
    u16 upper = (u16)(value >> 16);

    ext_store_palette16b(address, lower, pc);
    ext_store_palette16b((address + 2) & 0x3FF, upper, pc);
}

void function_cc ext_store_vram8(u32 address, u32 value, u32 pc) {
    address &= 0x1FFFF;
    vram[address] = (u8)value;
}

void function_cc ext_store_vram16(u32 address, u32 value, u32 pc) {
    address &= 0x1FFFF;
    *(u16 *)&vram[address] = (u16)value;
}

void function_cc ext_store_vram32(u32 address, u32 value, u32 pc) {
    address &= 0x1FFFF;

    if (address >= 0x18000) {
        address -= 0x8000;
    }

    *(u32 *)&vram[address] = value;
}

void function_cc ext_store_oam8(u32 address, u32 value, u32 pc) {
    oam_update = 1;
    address &= 0x3FF;
    oam_ram[address] = (u8)value;
}

void function_cc ext_store_oam16(u32 address, u32 value, u32 pc) {
    oam_update = 1;
    address &= 0x3FF;
    *(u16 *)&oam_ram[address] = (u16)value;
}

void function_cc ext_store_oam32(u32 address, u32 value, u32 pc) {
    oam_update = 1;
    address &= 0x3FF;
    *(u32 *)&oam_ram[address] = value;
}

void function_cc ext_store_eeprom(u32 address, u32 value, u32 pc) {
    write_eeprom(address, value); // replace with your EEPROM logic
}



void function_cc smc_write(u32 address) {
    // You can optionally log or track the address if needed
    flush_translation_cache_ram();
}

u32 function_cc collapse_flags_no_update(void) {
    u32 flags = 0;

    flags |= (sh4_reg_base[REG_N_FLAG] & 1) << 31;
    flags |= (sh4_reg_base[REG_Z_FLAG] & 1) << 30;
    flags |= (sh4_reg_base[REG_C_FLAG] & 1) << 29;
    flags |= (sh4_reg_base[REG_V_FLAG] & 1) << 28;
    flags |= (sh4_reg_base[REG_CPSR] & 0xFF);  // preserve mode bits

    return flags;
}

// Collapse flags and write to REG_CPSR
void function_cc collapse_flags_update(void) {
    sh4_reg_base[REG_CPSR] = collapse_flags_no_update();
}

void function_cc collapse_flags(void) {
    sh4_reg_base[REG_CPSR] = collapse_flags_no_update();
}

static inline void extract_flag(int shift, int offset) {
    sh4_reg_base[offset] = (sh4_reg_base[REG_CPSR] >> shift) & 1;
}

// Extract all N/Z/C/V flags from REG_CPSR into their respective registers
void function_cc extract_flags(void) {
    extract_flag(31, REG_N_FLAG);
    extract_flag(30, REG_Z_FLAG);
    extract_flag(29, REG_C_FLAG);
    extract_flag(28, REG_V_FLAG);
}



void sh4_indirect_branch_arm(u32 address) {
    sh4_reg_base=reg;
    SH4_LOG(DBG_INFO, "sh4_indirect_branch_arm: Entry with address=%08x, REG[REG_PC]=%08x\n", address, sh4_reg_base[REG_PC]);
        // sh4_reg_base[REG_PC] = address;  // reg[REG_PC]
        u8* target = block_lookup_address_arm(address) - block_prologue_size;
        u32 cycles = sh4_reg_cycles;
        asm volatile (
            "jsr @%0\n\t"
            "nop\n\t"
            : : "r"(target) : "pr", "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );

}

void sh4_indirect_branch_thumb(u32 address) {
    log_arm_state("sh4_indirect_branch_thumb");
    u8* target = block_lookup_address_thumb(address);
    if ((u32)target < 0x8c000000 || (u32)target > 0x8d000000 || !target) {
        SH4_LOG(DBG_ERROR, "Invalid/no block target %p for address=%08x\n", target, address);
        sh4_reg_base[CPU_HALT_STATE] = 1;  // reg[CPU_HALT_STATE]
        return;
    }
    u32 cycles = sh4_reg_cycles;
    asm volatile (
        "jsr @%0\n\t"
        "nop\n\t"
        : : "r"(target) : "pr", "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    );
}

void sh4_indirect_branch_dual(u32 address) {
    log_arm_state("sh4_indirect_branch_dual");
    u32 cpsr = sh4_reg_base[REG_CPSR];  // reg[REG_CPSR]
    u8* target = (cpsr & 0x20) ? block_lookup_address_thumb(address)
                               : block_lookup_address_arm(address);
    target -= block_prologue_size;
    if ((u32)target < 0x8c000000 || (u32)target > 0x8d000000 || !target) {
        SH4_LOG(DBG_ERROR, "Invalid/no block target %p for address=%08x\n", target, address);
        sh4_reg_base[CPU_HALT_STATE] = 1;  // reg[CPU_HALT_STATE]
        return;
    }
    sh4_reg_base[REG_PC] = address;  // reg[REG_PC]
    u32 cycles = sh4_reg_cycles;
    asm volatile (
        "jsr @%0\n\t"
        "nop\n\t"
        : : "r"(target) : "pr", "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    );
}

// // Helper functions
// void collapse_flags() {
//     reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) |
//                     (reg[REG_Z_FLAG] << 30) |
//                     (reg[REG_C_FLAG] << 29) |
//                     (reg[REG_V_FLAG] << 28) |
//                     (reg[REG_CPSR] & 0xFF);
// }

// void extract_flags() {
//     reg[REG_N_FLAG] = (reg[REG_CPSR] >> 31) & 1;
//     reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 1;
//     reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 1;
//     reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 1;
//      printf("Extracted flags: N=%d, Z=%d, C=%d, V=%d\n", reg[REG_N_FLAG], reg[REG_Z_FLAG], reg[REG_C_FLAG], reg[REG_V_FLAG]);
// }

// Flag update helpers
// static inline void update_flags_nz(u32 result) {
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
// }

// static inline void update_flags_add(u32 rm, u32 rn, u32 result) {
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = (u64)rn + (u64)rm > 0xFFFFFFFFU;
//     sh4_reg_base[REG_V_FLAG] = ((rn ^ result) & (rm ^ result)) >> 31;
// }

// static inline void update_flags_sub(u32 rm, u32 rn, u32 result) {
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = rn >= rm; // No borrow
//     sh4_reg_base[REG_V_FLAG] = ((rn ^ rm) & (rn ^ result)) >> 31;
// }

// u32 function_cc execute_clz(u32 rm) {
//     u32 count = 0;
//     while (rm) {
//         count++;
//         rm >>= 1;
//     }
//     return 32 - count;
// }



void function_cc execute_store_u8(u32 address, u32 value, u32 pc) {
    sh4_reg_base[REG_PC] = pc;

    // If address is outside normal RAM range, use external handler
    if ((address & 0xF0000000) != 0) {
        ext_store_u8(address, (u8)value, pc);
        return;
    }

    u32 page = address >> 15;
    u8 *map = memory_map_write[page];
    if (!map) {
        ext_store_u8(address, (u8)value, pc);
        return;
    }

    u32 offset = address & 0x7FFF;
    map[offset] = (u8)value;

    if (map[offset - 0x8000] != 0)
        smc_write(address);
}

void function_cc execute_store_u16(u32 address, u32 value, u32 pc) {
    address &= ~1;
    sh4_reg_base[REG_PC] = pc;

    if ((address & 0xF0000000) != 0) {
        ext_store_u16(address, (u16)value, pc);
        return;
    }

    u32 page = address >> 15;
    u8 *map = memory_map_write[page];
    if (!map) {
        ext_store_u16(address, (u16)value, pc);
        return;
    }

    u32 offset = address & 0x7FFF;
    *(u16 *)(map + offset) = (u16)value;

    if (*(u16 *)(map + offset - 0x8000) != 0)
        smc_write(address);
}

void function_cc execute_store_u32(u32 address, u32 value, u32 pc) {
    address &= ~3;
    sh4_reg_base[REG_PC] = pc;

    if ((address & 0xF0000000) != 0) {
        ext_store_u32(address, value, pc);
        return;
    }

    u32 page = address >> 15;
    u8 *map = memory_map_write[page];
    if (!map) {
        ext_store_u32(address, value, pc);
        return;
    }

    u32 offset = address & 0x7FFF;
    *(u32 *)(map + offset) = value;

    if (*(u32 *)(map + offset - 0x8000) != 0)
        smc_write(address);
}





// u32 execute_read_cpsr(void) {
//     collapse_flags();
//     return reg[REG_CPSR];
// }

// u32 execute_read_spsr(void) {
//     return spsr[reg[REG_CPSR] & 0x1F];
// }

// void execute_store_cpsr(u32 new_cpsr, u32 store_mask) {
//     reg[REG_CPSR] = (reg[REG_CPSR] & ~store_mask) | (new_cpsr & store_mask);
//     extract_flags();
// }

// void execute_store_spsr(u32 new_spsr, u32 store_mask) {
//     u32 mode = reg[REG_CPSR] & 0x1F;
//     spsr[mode] = (spsr[mode] & ~store_mask) | (new_spsr & store_mask);
// }

// u32 execute_spsr_restore(u32 address) {
//     SH4_LOG(DBG_INFO, "execute_spsr_restore: Called with address=%08x\n", address);
//     reg[REG_CPSR] = spsr[reg[CPU_MODE]];
//     if(reg[REG_CPSR] & 0x20) address |= 0x01;
//     return address;
//   }

// void execute_swi(u32 pc) {
//     u32 swi_num = (pc - 2) & 0xFF;
//     SH4_LOG(DBG_INFO, "SWI %u at PC %08x\n", swi_num, pc);
//     sh4_reg_base[REG_PC] = pc;  // reg[REG_PC]
//     if (swi_hle_handle[swi_num]) {
//         SH4_LOG(DBG_INFO, "HLE handling SWI %u\n", swi_num);
//         if (swi_num == 0x06) {
//             u32 r0 = sh4_reg_base[0];  // reg[0]
//             u32 r1 = sh4_reg_base[1];  // reg[1]
//             s32 num = r0;
//             s32 den = r1;
//             sh4_reg_base[0] = num / den;      // reg[0]
//             sh4_reg_base[1] = num % den;      // reg[1]
//             sh4_reg_base[3] = abs(num / den); // reg[3]
//             u32 new_pc = pc + 2;
//             sh4_reg_base[REG_PC] = new_pc;  // reg[REG_PC]
//             return;
//         }
//     }
//     collapse_flags();
//     u32 cycles = sh4_reg_cycles;
//     update_gba(cycles);
// }


// Bitwise ops
// u32 function_cc execute_and(u32 rm, u32 rn) { return rm & rn; }
// u32 function_cc execute_ands(u32 rm, u32 rn) {
//     u32 result = rm & rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_eor(u32 rm, u32 rn) { return rm ^ rn; }
// u32 function_cc execute_eors(u32 rm, u32 rn) {
//     u32 result = rm ^ rn;
//     update_flags_nz(result);
//     return result;
// }

// // Arithmetic ops
// u32 function_cc execute_sub(u32 rm, u32 rn) { return rn - rm; }
// u32 function_cc execute_subs(u32 rm, u32 rn) {
//     u32 result = rn - rm;
//     update_flags_sub(rm, rn, result);
//     return result;
// }
// u32 function_cc execute_rsb(u32 rm, u32 rn) { return rm - rn; }
// u32 function_cc execute_rsbs(u32 rm, u32 rn) {
//     u32 result = rm - rn;
//     update_flags_sub(rn, rm, result);
//     return result;
// }
// u32 function_cc execute_add(u32 rm, u32 rn) { return rn + rm; }

// u32 function_cc execute_adds(u32 rm, u32 rn) {
//     u32 result = rn + rm;
//     update_flags_add(rm, rn, result);
//     return result;
// }
// u32 function_cc execute_adc(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     return rn + rm + carry;
// }
// u32 function_cc execute_adcs(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     u32 result = rn + rm + carry;
//     reg[REG_N_FLAG] = (result >> 31) & 1;
//     reg[REG_Z_FLAG] = (result == 0);
//     reg[REG_C_FLAG] = (u64)rn + (u64)rm + carry > 0xFFFFFFFFU;
//     reg[REG_V_FLAG] = ((rn ^ result) & (rm ^ result)) >> 31;
//     return result;
// }
// u32 function_cc execute_sbc(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     return rn - rm - !carry;
// }
// u32 function_cc execute_sbcs(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     u32 result = rn - rm - !carry;
//     reg[REG_N_FLAG] = (result >> 31) & 1;
//     reg[REG_Z_FLAG] = (result == 0);
//     reg[REG_C_FLAG] = (u64)rn >= (u64)rm + !carry;
//     reg[REG_V_FLAG] = ((rn ^ rm) & (rn ^ result)) >> 31;
//     return result;
// }
// u32 function_cc execute_rsc(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     return rm - rn - !carry;
// }
// u32 function_cc execute_rscs(u32 rm, u32 rn) {
//     u32 carry = reg[REG_C_FLAG];
//     u32 result = rm - rn - !carry;
//     reg[REG_N_FLAG] = (result >> 31) & 1;
//     reg[REG_Z_FLAG] = (result == 0);
//     reg[REG_C_FLAG] = (u64)rm >= (u64)rn + !carry;
//     reg[REG_V_FLAG] = ((rm ^ rn) & (rm ^ result)) >> 31;
//     return result;
// }

// // More bitwise
// u32 function_cc execute_orr(u32 rm, u32 rn) { return rm | rn; }
// u32 function_cc execute_orrs(u32 rm, u32 rn) {
//     u32 result = rm | rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_mov(u32 rm) { 
//     return rm; }
// u32 function_cc execute_movs(u32 rm) {
//     update_flags_nz(rm);
//     return rm;
// }
// u32 function_cc execute_bic(u32 rm, u32 rn) { return rn & ~rm; }
// u32 function_cc execute_bics(u32 rm, u32 rn) {
//     u32 result = rn & ~rm;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_mvn(u32 rm) { return ~rm; }
// u32 function_cc execute_mvns(u32 rm) {
//     u32 result = ~rm;
//     update_flags_nz(result);
//     return result;
// }

// // Multiply
// u32 function_cc execute_mul(u32 rm, u32 rs) { return rm * rs; }
// u32 function_cc execute_muls(u32 rm, u32 rs) {
//     u32 result = rm * rs;
//     update_flags_nz(result);
//     return result;
// }

uint32_t reg_rv_lo;
uint32_t reg_rv_hi;

uint32_t function_cc execute_mul_long_add_u64(uint32_t rs, uint32_t rdlo, uint32_t rdhi) {
    uint64_t a = (uint64_t)rs;
    uint64_t b = ((uint64_t)rdhi << 32) | rdlo;
    uint64_t result = a + b;

    reg_rv_lo = (uint32_t)(result & 0xFFFFFFFF);
    reg_rv_hi = (uint32_t)(result >> 32);

    return reg_rv_lo;
}

uint32_t function_cc execute_mul_long_add_s64(int32_t rs, int32_t rdlo, int32_t rdhi) {
    int64_t a = (int64_t)rs;
    int64_t b = ((int64_t)rdhi << 32) | (uint32_t)rdlo;
    int64_t result = a + b;

    reg_rv_lo = (uint32_t)(result & 0xFFFFFFFF);
    reg_rv_hi = (uint32_t)(result >> 32);

    return reg_rv_lo;
}


// u32 function_cc execute_mul_long_flags(u32 dest_lo, u32 dest_hi) {
//     reg[REG_Z_FLAG] = (dest_lo == 0 && dest_hi == 0); // Z = 1 if result is zero
//     reg[REG_N_FLAG] = (dest_hi >> 31) & 1;           // N = 1 if high bit is set
//     // C and V flags are not modified (left as-is, consistent with ARMv4T)
//     return 0; // Return value not critical for flag update
// }


// u32 function_cc execute_mul_long(u32 rm, u32 rs) {
//     // Only low 32 bits for now; high 32 ignored
//     return rm * rs;// Bitwise ops
// u32 function_cc execute_and(u32 rm, u32 rn) { return rm & rn; }
// u32 function_cc execute_ands(u32 rm, u32 rn) {
//     u32 result = rm & rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_eor(u32 rm, u32 rn) { return rm ^ rn; }
// u32 function_cc execute_eors(u32 rm, u32 rn) {
//     u32 result = rm ^ rn;
//     update_flags_nz(result);
//     return result;
// }

// // Arithmetic ops
// u32 function_cc execute_sub(u32 rm, u32 rn) { return rn - rm; }
// u32 function_cc execute_subs(u32 rm, u32 rn) {
//     u32 result = rn - rm;
//     update_flags_sub(rm, rn, result);
//     return result;
// }
// u32 function_cc execute_rsb(u32 rm, u32 rn) { return rm - rn; }
// u32 function_cc execute_rsbs(u32 rm, u32 rn) {
//     u32 result = rm - rn;
//     update_flags_sub(rn, rm, result);
//     return result;
// }
// u32 function_cc execute_add(u32 rm, u32 rn) { return rn + rm; }

// u32 function_cc execute_adds(u32 rm, u32 rn) {
//     u32 result = rn + rm;
//     update_flags_add(rm, rn, result);
//     return result;
// }
// u32 function_cc execute_adc(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     return rn + rm + carry;
// }
// u32 function_cc execute_adcs(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     u32 result = rn + rm + carry;
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = (u64)rn + (u64)rm + carry > 0xFFFFFFFFU;
//     sh4_reg_base[REG_V_FLAG] = ((rn ^ result) & (rm ^ result)) >> 31;
//     return result;
// }
// u32 function_cc execute_sbc(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     return rn - rm - !carry;
// }
// u32 function_cc execute_sbcs(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     u32 result = rn - rm - !carry;
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = (u64)rn >= (u64)rm + !carry;
//     sh4_reg_base[REG_V_FLAG] = ((rn ^ rm) & (rn ^ result)) >> 31;
//     return result;
// }
// u32 function_cc execute_rsc(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     return rm - rn - !carry;
// }
// u32 function_cc execute_rscs(u32 rm, u32 rn) {
//     u32 carry = sh4_reg_base[REG_C_FLAG];
//     u32 result = rm - rn - !carry;
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = (u64)rm >= (u64)rn + !carry;
//     sh4_reg_base[REG_V_FLAG] = ((rm ^ rn) & (rm ^ result)) >> 31;
//     return result;
// }

// // More bitwise
// u32 function_cc execute_orr(u32 rm, u32 rn) { return rm | rn; }
// u32 function_cc execute_orrs(u32 rm, u32 rn) {
//     u32 result = rm | rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_mov(u32 rm) { 
//     SH4_LOG(DBG_INFO, "execute_mov: rm=%08x\n", rm);
//     return rm; }
// u32 function_cc execute_movs(u32 rm) {
//     update_flags_nz(rm);
//     return rm;
// }
// u32 function_cc execute_bic(u32 rm, u32 rn) { return rn & ~rm; }
// u32 function_cc execute_bics(u32 rm, u32 rn) {
//     u32 result = rn & ~rm;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_mvn(u32 rm) { return ~rm; }
// u32 function_cc execute_mvns(u32 rm) {
//     u32 result = ~rm;
//     update_flags_nz(result);
//     return result;
// }

// // Multiply
// u32 function_cc execute_mul(u32 rm, u32 rs) { 
//     SH4_LOG(DBG_INFO, "execute_mul: rm=%08x, rs=%08x\n", rm, rs);
//     return rm * rs; }
// u32 function_cc execute_muls(u32 rm, u32 rs) {
//     u32 result = rm * rs;
//     update_flags_nz(result);
//     return result;
// }

// u32 function_cc execute_mul_long_flags(u32 dest_lo, u32 dest_hi) {
//     sh4_reg_base[REG_Z_FLAG] = (dest_lo == 0 && dest_hi == 0); // Z = 1 if result is zero
//     sh4_reg_base[REG_N_FLAG] = (dest_hi >> 31) & 1;           // N = 1 if high bit is set
//     // C and V flags are not modified (left as-is, consistent with ARMv4T)
//     return 0; // Return value not critical for flag update
// }


// u32 function_cc execute_mul_long(u32 rm, u32 rs) {
//     // Only low 32 bits for now; high 32 ignored
//     return rm * rs;
// }

// u32 function_cc execute_mla(u32 rm, u32 rs, u32 rn) {
//     return rm * rs + rn;
// }

// u32 function_cc execute_mlas(u32 rm, u32 rs, u32 rn) {
//     u32 result = rm * rs + rn;
//     update_flags_nz(result);
//     return result;
// }

u32 function_cc execute_mul_long_u64(u32 rm, u32 rs) {
    u64 result = (u64)rm * (u64)rs;
    u32 low = result & 0xFFFFFFFF;
    u32 high = result >> 32;
    __asm__ volatile (
        "mov %0, r0\n\t"
        "mov %1, r1\n\t"
        : : "r" (low), "r" (high) : "r0", "r1"
    );
    // Add flag update for UMULLS
    sh4_reg_base[REG_Z_FLAG] = (low == 0 && high == 0);
    sh4_reg_base[REG_N_FLAG] = (high >> 31) & 1;
    return low;
}

// // Unsigned Multiply-Accumulate Long (UMLAL): rdhi:rdlo += rm * rs
// // u32 function_cc execute_umlal(u32 rm, u32 rs, u32 rdlo, u32 rdhi) {
// //     u64 current = ((u64)rdhi << 32) | rdlo;
// //     u64 product = (u64)rm * (u64)rs;
// //     u64 result = current + product;
// //     u32 low = result & 0xFFFFFFFF;
// //     u32 high = result >> 32;
// //     __asm__ volatile (
// //         "mov %0, r0\n\t"
// //         "mov %1, r1\n\t"
// //         : : "r" (low), "r" (high) : "r0", "r1"
// //     );
// //     return low;
// // }

u32 function_cc execute_mul_long_s64(u32 rm, u32 rs) {
    s64 result = (s64)(s32)rm * (s64)(s32)rs;
    u32 low = result & 0xFFFFFFFF;
    u32 high = result >> 32;
    __asm__ volatile (
        "mov %0, r0\n\t"
        "mov %1, r1\n\t"
        : : "r" (low), "r" (high) : "r0", "r1"
    );
    return low;
}

// // u32 function_cc execute_smlal(u32 rm, u32 rs, u32 rdlo, u32 rdhi) {
// //     s64 current = ((s64)rdhi << 32) | rdlo;
// //     s64 product = (s64)(s32)rm * (s64)(s32)rs;
// //     s64 result = current + product;
// //     u32 low = result & 0xFFFFFFFF;
// //     u32 high = result >> 32;
// //     __asm__ volatile (
// //         "mov %0, r0\n\t"
// //         "mov %1, r1\n\t"
// //         : : "r" (low), "r" (high) : "r0", "r1"
// //     );
// //     return low;
// // }

// // // 64-bit unsigned addition
// // u64 function_cc execute_u64_add(u64 rm, u64 rn) {
// //     return rm + rn;
// // }

// // // 64-bit unsigned operation (placeholder, adjust as needed)
// // u64 function_cc execute_u64(u64 rm, u64 rn) {
// //     return rm; // Placeholder, adjust as needed
// // }

// // // 64-bit signed addition
// // s64 function_cc execute_s64_add(s64 rm, s64 rn) {
// //     return rm + rn;
// // }

// // // 64-bit signed operation (placeholder, adjust as needed)
// // s64 function_cc execute_s64(s64 rm, s64 rn) {
// //     return rm; // Placeholder, adjust as needed
// // }

// // Test ops (flags only)
// u32 function_cc execute_tst(u32 rm, u32 rn) {
//     u32 result = rm & rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_teq(u32 rm, u32 rn) {
//     u32 result = rm ^ rn;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_cmp(u32 rm, u32 rn) {
//     // sh4_reg_base = reg; 
//     u32 result = rn - rm;
//     sh4_reg_base[REG_N_FLAG] = (result >> 31) & 1;
//     sh4_reg_base[REG_Z_FLAG] = (result == 0);
//     sh4_reg_base[REG_C_FLAG] = rn >= rm;
//     sh4_reg_base[REG_V_FLAG] = ((rn ^ rm) & (rn ^ result)) >> 31;

//     SH4_LOG(DBG_INFO, "execute_cmp: rn=%08x, rm=%08x, result=%08x, N=%d, Z=%d, C=%d, V=%d, PC=%08x, LR=%08x\n",
//         rn, rm, result,
//         sh4_reg_base[REG_N_FLAG],
//         sh4_reg_base[REG_Z_FLAG],
//         sh4_reg_base[REG_C_FLAG],
//         sh4_reg_base[REG_V_FLAG],
//         sh4_reg_base[REG_PC],
//         sh4_reg_base[REG_LR]);
        
// }

// u32 function_cc execute_cmn(u32 rm, u32 rn) {
//     u32 result = rn + rm;
//     update_flags_add(rm, rn, result);
//     return result;
// }

// // Shifts
// u32 function_cc execute_lsl_imm(u32 value, u32 shift) { return value << shift; }
// u32 function_cc execute_lsr_imm(u32 value, u32 shift) { return value >> shift; }
// u32 function_cc execute_asr_imm(u32 value, u32 shift) { return (s32)value >> shift; }
// u32 function_cc execute_lsl_reg(u32 value, u32 shift) {
//     if (shift == 0) return value; // C unchanged
//     else if (shift <= 32) {
//         u32 result = value << shift;
//         sh4_reg_base[REG_C_FLAG] = (value >> (32 - shift)) & 1;
//         update_flags_nz(result);
//         return result;
//     } else {
//         sh4_reg_base[REG_C_FLAG] = 0;
//         update_flags_nz(0);
//         return 0;
//     }
// }

// u32 function_cc execute_ror_imm(u32 value, u32 shift) {
//     u32 result;
//     if (shift != 0) {
//         // Standard ROR: (value >> shift) | (value << (32 - shift))
//         sh4_reg_base[REG_C_FLAG] = (value >> (shift - 1)) & 0x01; // Carry from last bit shifted out
//         result = (value >> shift) | (value << (32 - shift));
//     } else {
//         // RRX: Rotate Right with Extend
//         u32 c_flag = sh4_reg_base[REG_C_FLAG];         // Old Carry
//         sh4_reg_base[REG_C_FLAG] = value & 0x01;       // New Carry from LSb
//         result = (value >> 1) | (c_flag << 31); // Shift right, insert Carry
//     }
//     update_flags_nz(result); // N = sign, Z = zero
//     return result;
// }

// u32 function_cc execute_lsr_reg(u32 value, u32 shift) {
//     u32 result = value >> shift;
//     sh4_reg_base[REG_C_FLAG] = (shift > 0 && shift <= 32) ? (value >> (shift - 1)) & 1 : 0;
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_asr_reg(u32 value, u32 shift) {
//     u32 result = (s32)value >> shift;
//     sh4_reg_base[REG_C_FLAG] = (shift > 0 && shift <= 32) ? ((s32)value >> (shift - 1)) & 1 : ((s32)value < 0);
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_ror_reg(u32 value, u32 shift) {
//     u32 result = (value >> shift) | (value << (32 - shift));
//     sh4_reg_base[REG_C_FLAG] = (shift > 0) ? (value >> (shift - 1)) & 1 : sh4_reg_base[REG_C_FLAG];
//     update_flags_nz(result);
//     return result;
// }
// u32 function_cc execute_neg(u32 rm) {
//     u32 result = -rm;
//     update_flags_sub(rm, 0, result);
//     return result;
// }

void log_hash_chain(u32 pc, u32 hash_target, u32* initial_block_ptr, u32 translation_recursion_level) {
    SH4_LOG(DBG_INFO, "Hash chain for PC=%08x, hash_target=%u, recursion_level=%u:\n",
            pc, hash_target, translation_recursion_level);
    if (!initial_block_ptr) {
        SH4_LOG(DBG_INFO, "  [empty] Chain is empty\n");
        return;
    }
    u32* ptr = initial_block_ptr;
    int i = 0;
    while (ptr) {
        u32 stored_pc = ptr[0];
        u8* block_addr = (u8*)(ptr + 2);
        u32* next_ptr = (u32*)ptr[1];
        SH4_LOG(DBG_INFO,
                "  [%02d] Stored PC=%08x%s | block_addr=%p | ptr=%p | next_ptr=%p\n",
                i++, stored_pc,
                (stored_pc == pc ? " (MATCH)" : ""),
                block_addr, ptr, next_ptr);
        ptr = next_ptr;
    }
}





void step_debug_sh4(u32 pc) {
    SH4_LOG(DBG_INFO, "Debug step at PC=%08x\n", pc);
    // Add debug logic here
}
