 /* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

 #ifndef SH4_EMIT_H 
 #define SH4_EMIT_H
 
 #include <kos.h>
//  #include "../common.h"
//  #include "../cpu.h" 
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long int u64;
typedef signed long long int s64;
#define function_cc
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

 #ifndef DEBUG_DYNAREC
 #define DEBUG_DYNAREC 0 // Default to off; enable with -DDEBUG_DYNAREC=1 in Makefile
 #endif
 
 #if DEBUG_DYNAREC
 #define SH4_LOG(level, fmt, ...) dbglog(level, fmt, ##__VA_ARGS__)
 #else
 #define SH4_LOG(level, fmt, ...) do { } while (0) // No-op in release
 #endif

//  #define LOG_CALL_SITE() \
//   SH4_LOG(DBG_INFO, "SH4 macro expanded at %s:%d in %s\n", __FILE__, __LINE__, __func__)

 extern void generate_indirect_branch_arm();
 extern void generate_indirect_branch_dual();
 extern uint32_t reg_rv_lo;
extern uint32_t reg_rv_hi;
 extern uint32_t  execute_mul_long_add_u64(uint32_t rs, uint32_t rdlo, uint32_t rdhi);
extern uint32_t  execute_mul_long_add_s64(int32_t rs, int32_t rdlo, int32_t rdhi);
// extern u8 *translation_ptr;
extern u32 sh4_update_gba(u32 pc);
extern u32 execute_lookup_pc(u32 cycles);
// extern u8* current_block_start;
// Sets ARM PC in reg[15], no direct SH-4 register mapping
// extern void set_arm_pc(u32 value);
// extern  u32 get_arm_pc();
// extern  u32 reg_base;
// extern  u32 reg_cycles;
extern void dump_block(u8* block_address, size_t size) ;
extern void log_indirect_branch(u32 target, u32 block_start) ;
extern void log_sh4_registers(const char *context);
extern void log_sh4_registers_wrapper();
// Retrieves ARM PC from reg[15]
// extern u32 current_condition; //used for generate_indirect_branch_arm or generate_indirect_branch_dual
extern void log_hash_chain(u32 pc, u32 hash_target, u32 *initial_block_ptr, u32 translation_recursion_level);
extern void log_block_start();
extern void log_block_exit();
// Although these are defined as a function, don't call them as
// such (jump to it instead)
extern void sh4_indirect_branch_arm(u32 address);
extern void sh4_indirect_branch_thumb(u32 address);
extern void sh4_indirect_branch_dual(u32 address);

void step_debug_x86(u32 pc);

// extern void set_cpu_mode(cpu_mode_type new_mode);
extern u32 reg[64]; // Pointer to arm/emuregister array

// Literal pool structure
typedef struct {
  u16* movl_addr; // Address of MOV.L instruction
  u32 immediate;        // Immediate value
  u8 destination;        // Destination register (Rn)
} literal_entry;

static literal_entry literals[100]; // Adjust size as needed
static u32 literal_count = 0;
// extern u32 cycle_counter; 
#define MAX_BRANCH_PATCHES 8  // Adjust based on max expected branches per block
u8* branch_patch_list[MAX_BRANCH_PATCHES];
u32 branch_patch_count;

// SH4 register numbers (0-15 correspond to r0-r15)
typedef enum
{
  sh4_reg_number_r0,
  sh4_reg_number_r1,
  sh4_reg_number_r2,
  sh4_reg_number_r3,
  sh4_reg_number_r4,
  sh4_reg_number_r5,
  sh4_reg_number_r6,
  sh4_reg_number_r7,
  sh4_reg_number_r8,
  sh4_reg_number_r9,
  sh4_reg_number_r10,
  sh4_reg_number_r11,
  sh4_reg_number_r12,
  sh4_reg_number_r13,
  sh4_reg_number_r14,
  sh4_reg_number_r15
} sh4_reg_number;

#define sh4_emit_byte(value)                                                  \
SH4_LOG(DBG_INFO, "[GEN] Emit Byte: %02x @ %p\n", (u8)(value), translation_ptr); \
  *translation_ptr = value;                                                   \
  translation_ptr++                                                           \

#define sh4_emit_dword(value)                                                 \
SH4_LOG(DBG_INFO, "[GEN] Emit Dword: %02x @ %p\n", (u8)(value), translation_ptr); \
*((u32 *)translation_ptr) = value;                                          \
translation_ptr += 4 

#define sh4_emit_word(value) \
do { \
    SH4_LOG(DBG_INFO, "[GEN] Emit: %04x @ %p\n", (u16)(value), translation_ptr); \
    *((u16*)translation_ptr) = (value); \
    translation_ptr += 2; \
} while (0)


typedef enum
{
  sh4_mod_mem        = 0,
  sh4_mod_mem_disp8  = 1,
  sh4_mod_mem_disp32 = 2,
  sh4_mod_reg        = 3
} sh4_mod;

#define sh4_emit_mod_rm(mod, rm, spare)                                       \
  sh4_emit_byte((mod << 6) | (spare << 3) | rm)                               \

// #define sh4_emit_mem_op(dest, base, offset) \
//     if(offset == 0) \
//     { \
//       sh4_emit_mod_rm(sh4_mod_mem, base, dest); \
//     } \
//     else if(((s32)offset < 127) && ((s32)offset > -128)) \
//     { \
//       sh4_emit_mod_rm(sh4_mod_mem_disp8, base, dest); \
//       sh4_emit_byte((s8)offset); \
//     } \
//     else \
//     { \
//       sh4_emit_mod_rm(sh4_mod_mem_disp32, base, dest); \
//       sh4_emit_dword(offset); \
//     } \

#define sh4_emit_reg_op(dest, source)                                         \
  sh4_emit_mod_rm(sh4_mod_reg, source, dest)                                  \

  typedef enum {
    sh4_opcode_mov_reg_reg    = 0x6003,
    sh4_opcode_mov_reg_imm    = 0xE000,
    sh4_opcode_mov_reg_mem    = 0x2000,
    sh4_opcode_mov_mem_reg    = 0x6000,
    sh4_opcode_add_reg_reg    = 0x300C,
    sh4_opcode_add_reg_imm    = 0x7000,
    sh4_opcode_sub_reg_reg    = 0x3008,
    sh4_opcode_and_reg_reg    = 0x2009,
    sh4_opcode_or_reg_reg     = 0x200B,
    sh4_opcode_xor_reg_reg    = 0x200A,
    sh4_opcode_cmp_eq_reg     = 0x3000,
    sh4_opcode_tst_reg_reg    = 0x2008,
    sh4_opcode_shll_reg       = 0x4000,
    sh4_opcode_shlr_reg       = 0x4001,
    sh4_opcode_shar_reg       = 0x4004,
    sh4_opcode_shll1_reg      = 0x4000,
    sh4_opcode_shll2_reg      = 0x4008,
    sh4_opcode_shll8_reg      = 0x4018,
    sh4_opcode_shll16_reg     = 0x4028,
    sh4_opcode_shlr1_reg      = 0x4001,
    sh4_opcode_shlr2_reg      = 0x4009,
    sh4_opcode_shlr8_reg      = 0x4019,
    sh4_opcode_shlr16_reg     = 0x4029,
    sh4_opcode_shar1_reg      = 0x4004,
    sh4_opcode_shar2_reg      = 0x400C,
    sh4_opcode_shar8_reg      = 0x401C,
    sh4_opcode_shar16_reg     = 0x402C,
    sh4_opcode_bra            = 0xA000,
    sh4_opcode_bt             = 0x8900,
    sh4_opcode_bf             = 0x8B00,
    sh4_opcode_rts            = 0x000B,
    sh4_opcode_nop            = 0x0009,
    sh4_opcode_jmp            = 0x402B,
    sh4_opcode_cmp_pl         = 0x4015,
    sh4_opcode_ror_reg_imm    = 0x4024,
    sh4_opcode_adc_reg_rm     = 0x300E,
    sh4_opcode_and_rm_imm     = 0xC200,
    sh4_opcode_xor_rm_imm     = 0xCA00,
    sh4_opcode_test_rm_imm    = 0xC800,
    sh4_opcode_mul_eax_rm     = 0x0070,
    sh4_opcode_imul_eax_rm    = 0x0070,
    sh4_opcode_push_imm       = 0x0000, // Stub: SH-4 lacks direct push
    sh4_opcode_call_offset    = 0xA000, // Use BRA as approximation
    sh4_opcode_ret            = 0x000B  // RTS
  } sh4_opcodes;

  typedef enum {
    sh4_condition_eq  = 0x01, // T=1 (equal)
    sh4_condition_ne  = 0x00, // T=0 (not equal)
    sh4_condition_z   = 0x01, // T=1 (zero, same as eq)
    sh4_condition_nz  = 0x00  // T=0 (non-zero, same as ne)
  } sh4_condition_codes;

// New: Shift right arithmetic with fixed amounts
#define sh4_emit_shar_reg_imm(dest, imm) \
   { \
    u32 shift = (imm); \
    if (shift >= 16) { \
      sh4_emit_word(sh4_opcode_shar16_reg | ((dest) << 8)); \
      shift -= 16; \
    } \
    if (shift >= 8) { \
      sh4_emit_word(sh4_opcode_shar8_reg | ((dest) << 8)); \
      shift -= 8; \
    } \
    if (shift >= 2) { \
      sh4_emit_word(sh4_opcode_shar2_reg | ((dest) << 8)); \
      shift -= 2; \
    } \
    if (shift >= 1) { \
      sh4_emit_word(sh4_opcode_shar1_reg | ((dest) << 8)); \
    } \
    if (shift > 0) { \
      SH4_LOG(DBG_ERROR, "Shift %d not fully handled\n", imm); \
    } \
  } 



#define sh4_relative_offset(source, offset, next)                             \
((u32)offset - ((u32)source + next))                                        \

#define sh4_unequal_operands(op_a, op_b)                                      \
  (op_a != op_b)                            \

// #define sh4_emit_opcode_1b_reg(opcode, dest, source)                          \
//   sh4_emit_byte(sh4_opcode_##opcode);                                         \
//   sh4_emit_reg_op(dest, source)            \
                                                                         

// #define sh4_emit_opcode_1b_mem(opcode, dest, base, offset)                    \
// {                                                                             \
//   sh4_emit_byte(sh4_opcode_##opcode);                                         \
//   sh4_emit_mem_op(dest, base, offset);      \
// }                                                                             \

// #define sh4_emit_opcode_1b(opcode, reg)                                       \
//   sh4_emit_byte(sh4_opcode_##opcode | reg)                   \

// #define sh4_emit_opcode_1b_ext_reg(opcode, dest)                              \
//   sh4_emit_byte(sh4_opcode_##opcode & 0xFF);                                  \
//   sh4_emit_reg_op(sh4_opcode_##opcode >> 8, dest)            \


// #define sh4_emit_opcode_1b_ext_mem(opcode, base, offset)                      \
//   sh4_emit_byte(sh4_opcode_##opcode & 0xFF);                                  \
//   sh4_emit_mem_op(sh4_opcode_##opcode >> 8, base, offset)    \


#define sh4_emit_mov_reg_mem(dest, base, offset) \
  SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_mem: MOV r%d -> @r%d + %d\n", dest, base, offset); \
  sh4_emit_word(sh4_opcode_mov_reg_mem | ((base) << 8) | ((dest) << 4) | ((offset) & 0xF))
                
#define sh4_emit_mov_mem_reg(source, base, offset) \
  SH4_LOG(DBG_INFO, "sh4_emit_mov_mem_reg: MOV @r%d + %d -> r%d\n", base, offset, source); \
  sh4_emit_word(sh4_opcode_mov_mem_reg | ((base) << 8) | ((source) << 4) | ((offset) & 0xF))

#define sh4_emit_cmp_pl(reg) \
  sh4_emit_word(sh4_opcode_cmp_pl | ((reg) << 8))


#define sh4_emit_mov_reg_reg(dest, source) \
  do { \
      SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_reg: MOV r%d -> r%d\n", source, dest); \
      u16 opcode = sh4_opcode_mov_reg_reg | (dest << 8) | (source << 4); \
      sh4_emit_word(opcode); \
      SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_reg: Emitted opcode: %04x\n", opcode); \
  } while (0)

  #define sh4_emit_mov_reg_imm(dest, imm) do { \
    if ((imm) >= -128 && (imm) <= 127) { \
        SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_imm: MOV: emitting immediate %d to r%d\n", (int)(imm), dest); \
        sh4_emit_word(0xE000 | ((dest) << 8) | ((imm) & 0xFF)); \
    } else { \
        if (literal_count >= 100) { \
            SH4_LOG(DBG_ERROR, "sh4_emit_mov_reg_imm: Literal pool overflow\n"); \
            reg[CPU_HALT_STATE] = 1; \
        } else { \
            SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_imm: emitting MOV.L @disp, r%d, stored in Literal[%d]\n", dest, literal_count); \
            sh4_emit_word(0xD000 | ((dest) << 8)); /* MOV.L @disp, Rn */ \
            u16* movl_addr = (u16*)(translation_ptr - 2); /* ← correct spot */ \
            literals[literal_count].movl_addr = movl_addr; \
            literals[literal_count].immediate = (imm); \
            literals[literal_count].destination = (dest); \
            literal_count++; \
        } \
    } \
} while (0)




// #define sh4_emit_set_reg_mem_immediate_now(dest, reg_index)        \
// do {                                                               \
//     u32 _imm_val = (dest);                                         \
//     u32 _reg = (reg_index);                                        \
//     if (_imm_val <= 127) {                                         \
//         /* MOV #imm, Rn */                                         \
//         sh4_emit_word(0xE000 | ((_reg) << 8) | (_imm_val));        \
//     } else {                                                       \
//         sh4_emit_mov_reg_imm_force(_reg, _imm_val);                \
//     }                                                              \
//     sh4_emit_mov_mem_reg(_reg, reg_base, (_reg));              \
// } while (0)



// #define sh4_emit_mov_reg_imm_force(dest, imm) do { \
//   u16* movl_addr = (u16*)translation_ptr; \
//   sh4_emit_word(0xD000 | ((dest) << 8)); /* MOV.L @disp, Rn */ \
//   *((u32*)translation_ptr) = (imm); \
//   translation_ptr += 4; \
// } while (0)


// #define sh4_emit_mov_mem_imm(imm, base, offset)                               \
//   SH4_LOG(DBG_INFO, "MOV: emitting immediate %d to @r%d + %d\n", (int)(imm), base, offset); \
//   sh4_emit_opcode_1b_ext_mem(mov_rm_imm, base, offset);                       \
//   sh4_emit_dword(imm)                                                         \

#define sh4_emit_sar_reg_imm(dest, imm) \
  {                                     \
    u32 shift = (imm);                  \
    for (u32 i = 0; i < shift; i++) {   \
      sh4_emit_word(sh4_opcode_shar_reg | ((dest) << 8)); \
    }                                   \
  }

#define sh4_emit_ror_reg_imm(dest, imm) \
  sh4_emit_mov_reg_imm(reg_a1, imm);    \
  sh4_emit_word(0x4005 | ((dest) << 8)); /* ROTR Rn (needs adjustment for imm) */                                                         \

#define sh4_emit_add_reg_reg(dest, source) \
  SH4_LOG(DBG_INFO, "ADD: emitting ADD r%d + r%d -> r%d\n", source, dest, dest); \
  sh4_emit_word(sh4_opcode_add_reg_reg | ((dest) << 8) | ((source) << 4))

#define sh4_emit_adc_reg_reg(dest, source) \
  sh4_emit_word(0x300C | ((dest) << 8) | ((source) << 4))

#define sh4_emit_sub_reg_reg(dest, source)                                    \
  sh4_emit_word(sh4_opcode_sub_reg_reg  | ((dest) << 8) | ((source) << 4))    \

#define sh4_emit_or_reg_reg(dest, source)                                     \
  sh4_emit_word(sh4_opcode_or_reg_reg  | ((dest) << 8) | ((source) << 4))

#define sh4_emit_xor_reg_reg(dest, source)                                    \
  sh4_emit_word(sh4_opcode_xor_reg_reg | ((dest) << 8) | ((source) << 4))

#define sh4_emit_add_reg_imm(dest, imm)                                       \
do {                                                                        \
  int _imm = (imm);                                                         \
  if (_imm != 0) {                                                          \
    if (_imm >= -128 && _imm <= 127) {                                      \
      /* Use ADD immediate (1-byte signed) when possible */                 \
      sh4_emit_word(sh4_opcode_add_reg_imm | ((dest) << 8) | ((u8)_imm));  \
    } else {                                                                \
      /* For larger values, load into temp register and add */               \
      sh4_emit_mov_reg_imm(reg_a1, _imm);                                   \
      sh4_emit_add_reg_reg(dest, reg_a1);                                   \
    }                                                                       \
  }                                                                         \
} while (0)

#define generate_shift_right_arithmetic(ireg, imm)                            \
  sh4_emit_sar_reg_imm(ireg, imm)                                       \

        
#define sh4_emit_sub_reg_imm(dest, imm)                                       \
  if (imm != 0) { \
      u16 opcode = sh4_opcode_add_reg_imm | (dest << 8) | ((u8)(-imm)); \
      sh4_emit_word(opcode);  \
  } \

#define sh4_emit_xor_reg_imm(dest, imm) \
  sh4_emit_mov_reg_imm(reg_a1, imm);    \
  sh4_emit_xor_reg_reg(dest, reg_a1)

#define sh4_emit_test_reg_imm(dest, imm) \
  SH4_LOG(DBG_INFO, "[EMIT] TST R%d, #0x%04x at %p\n", dest, imm, translation_ptr); \
  sh4_emit_mov_reg_imm(reg_a1, imm);     \
  sh4_emit_tst_reg_reg(dest, reg_a1)        \

  #define SH4_EMIT_DEBUG(opname, value) \
  SH4_LOG(DBG_INFO, "[GEN] %-10s : %04x @ %p\n", #opname, (u16)(value), translation_ptr)

  #define sh4_emit_cmp_reg_reg(dest, source) \
  SH4_LOG(DBG_INFO, "[EMIT] CMP R%d, R%d at %p\n", dest, source, translation_ptr); \
  sh4_emit_word(0x3000 | ((dest) << 8) | ((source) << 4))

#define sh4_emit_test_reg_reg(dest, source) \
  SH4_LOG(DBG_INFO, "[EMIT] TST R%d, R%d at %p\n", dest, source, translation_ptr); \
  sh4_emit_word(sh4_opcode_tst_reg_reg | ((dest) << 8) | ((source) << 4))

#define sh4_emit_cmp_reg_imm(reg, imm) \
do { \
    SH4_LOG(DBG_INFO, "[EMIT] CMP R%d, #0x%04x at %p\n", reg, imm, translation_ptr); \
    /* First move immediate to temporary register (R0) */ \
    sh4_emit_mov_reg_imm(0, imm); \
    /* Then compare registers (CMP/EQ R0, Rn) */ \
    sh4_emit_word(sh4_opcode_cmp_eq_reg | (reg << 8)); \
} while (0)


// #define sh4_emit_mul_eax_reg(source) \
//   sh4_emit_mov_reg_reg(reg_a4, source); \
//   generate_function_call(execute_mul);  \
//   sh4_emit_mov_reg_reg(reg_rv, reg_a0) \

// #define sh4_emit_imul_eax_reg(source)                                         \
//   sh4_emit_opcode_1b_ext_reg(imul_eax_rm, source)                             \

// #define sh4_emit_idiv_eax_reg(source)                                         \
//   sh4_emit_opcode_1b_ext_reg(idiv_eax_rm, source)                             \

  #define sh4_emit_push_mem(base, offset) \
    if (offset == 0) { \
      /* MOV.L @base, reg_a1 */ \
      sh4_emit_word(0x6000 | ((base) << 8) | (reg_a1 << 4)); \
    } else { \
      /* MOV base, reg_a0 */ \
      sh4_emit_word(0x6003 | ((base) << 4) | (reg_a0 << 8)); \
      /* Add offset to reg_a0 */ \
      if ((offset) <= 0x7F && (offset) >= -0x80) { \
        /* ADD #imm, reg_a0 (8-bit immediate) */ \
        sh4_emit_word(0x7000 | (reg_a0 << 8) | ((u8)(offset))); \
      } else { \
        /* Load large offset into reg_a1 */ \
        sh4_emit_mov_reg_imm(reg_a1, offset); \
        /* ADD reg_a1, reg_a0 */ \
        sh4_emit_word(0x300C | (reg_a0 << 8) | (reg_a1 << 4)); \
      } \
      /* MOV.L @reg_a0, reg_a1 */ \
      sh4_emit_word(0x6000 | (reg_a0 << 8) | (reg_a1 << 4)); \
    } \
    /* ADD #-4, R15 */ \
    sh4_emit_word(0x7FEC); \
    /* MOV.L reg_a1, @R15 */ \
    sh4_emit_word(0x2002 | (reg_a1 << 4) | (15 << 8)); \

#define sh4_emit_push_imm(imm)                                                \
  SH4_LOG(DBG_INFO, "PUSH: emitting immediate %d\n", (int)(imm)); \
  sh4_emit_byte(sh4_opcode_push_imm);                                         \
  sh4_emit_dword(imm)                                                         \

#define sh4_emit_call_offset(relative_offset)                                 \
  sh4_emit_byte(sh4_opcode_call_offset);                                      \
  sh4_emit_dword(relative_offset)                                             \

#define sh4_emit_ret()                                                        \
  sh4_emit_byte(sh4_opcode_ret)                                               \

#define sh4_emit_lea_reg_mem(dest, base, offset)                              \
  sh4_emit_mov_reg_reg(dest, base);                                           \
  if ((offset) <= 0xFF && (offset) >= -0x80) {                                \
    sh4_emit_word(sh4_opcode_add_reg_imm | ((dest) << 8) | ((u8)(offset)));   \
  } else {                                                                    \
    sh4_emit_mov_reg_imm(reg_a1, offset);                                     \
    sh4_emit_add_reg_reg(dest, reg_a1);                                       \
  }

  #define sh4_emit_j_filler(condition, writeback_location) \
  do { \
      translation_ptr = (u8*)(((uintptr_t)translation_ptr + 3) & ~3); \
      *(writeback_location) = translation_ptr; \
      u16 opcode = (condition ? sh4_opcode_bt : sh4_opcode_bf); \
      sh4_emit_word(opcode); \
      sh4_emit_word(sh4_opcode_nop); \
      SH4_LOG(DBG_INFO, "Emitted %s at %p\n", condition ? "BT" : "BF", translation_ptr-4); \
  } while (0)




#define sh4_condition_code_ns  0x8B

#define sh4_emit_j_offset(condition, offset)                                  \
  sh4_emit_word((condition == sh4_condition_eq ? sh4_opcode_bt : sh4_opcode_bf) | \
                ((offset >> 1) & 0xFF));                                     \
  sh4_emit_word(sh4_opcode_nop)                                              \


#define sh4_emit_jmp_filler(writeback_location) \
do { \
    SH4_LOG(DBG_INFO, "sh4_emit_jmp_filler: emitting JMP filler at %p\n", translation_ptr); \
    u8 *jmp_ptr = translation_ptr; \
    sh4_emit_word(sh4_opcode_bra); \
    sh4_emit_word(sh4_opcode_nop); \
    (writeback_location) = jmp_ptr; \
    translation_ptr += 4; \
} while(0)


#define sh4_emit_jmp_offset(offset) \
  SH4_LOG(DBG_INFO, "sh4_emit_jmp_offset: emitting JMP %08x\n", offset); \
  sh4_emit_mov_reg_imm(reg_a0, offset); \
  sh4_emit_jmp_reg(reg_a0)                                                   \

#define sh4_emit_jmp_reg(reg) \
  SH4_LOG(DBG_INFO, "sh4_emit_jmp_reg: emitting JMP r%d\n", reg); \
  sh4_emit_word(sh4_opcode_jmp | ((reg) << 8))                               \

  #define reg_a0      sh4_reg_number_r0
  #define reg_a1      sh4_reg_number_r1
  #define reg_a2      sh4_reg_number_r2
  #define reg_a4      sh4_reg_number_r4 
  #define reg_rv      sh4_reg_number_r0
  #define reg_s0      sh4_reg_number_r4
  #define a0  sh4_reg_number_r0
  #define a1  sh4_reg_number_r1
  #define a2  sh4_reg_number_r2
  #define a3  sh4_reg_number_r3
  #define rv  sh4_reg_number_r0
  #define s0  sh4_reg_number_r4
  #define a4  sh4_reg_number_r4   
  #define a5 sh4_reg_number_r5
  #define a6 sh4_reg_number_r6
  #define a7 sh4_reg_number_r7

  #define reg_base    sh4_reg_number_r12
  #define reg_cycles  sh4_reg_number_r13


  #define generate_load_reg(ireg, reg_index) \
  SH4_LOG(DBG_INFO, "sh4_emit_mov_reg_mem: Loading R%d from reg[%d]\n", ireg, reg_index); \
  sh4_emit_mov_mem_reg(ireg, reg_base, reg_index) 

  #define generate_load_pc(ireg, new_pc) \
  do { \
      SH4_LOG(DBG_INFO, "generate_load_pc: %08x into R%d\n", new_pc, ireg); \
      sh4_emit_mov_reg_imm(ireg, new_pc); \
      sh4_emit_mov_mem_reg(ireg, reg_base, ireg);  \
  } while (0)

  #define generate_load_imm(ireg, imm) \
  SH4_LOG(DBG_INFO, "generate_load_imm: %d into R%d\n", (int)(imm), ireg); \
  sh4_emit_mov_reg_imm(ireg, imm)

  #define generate_store_reg(ireg, reg_index) \
  SH4_LOG(DBG_INFO, "generate_store_reg: Storing R%d to reg[%d]\n", ireg, reg_index); \
  sh4_emit_mov_reg_mem(ireg, reg_base, reg_index)


#define sh4_emit_shl_reg_imm(dest, imm)                                       \
  {                                                                           \
    u32 shift = (imm);                                                        \
    while (shift > 0) {                                                       \
      if (shift >= 16) {                                                      \
        sh4_emit_word(sh4_opcode_shll16_reg | ((dest) << 8));                 \
        shift -= 16;                                                          \
      } else if (shift >= 8) {                                                \
        sh4_emit_word(sh4_opcode_shll8_reg | ((dest) << 8));                  \
        shift -= 8;                                                           \
      } else if (shift >= 2) {                                                \
        sh4_emit_word(sh4_opcode_shll2_reg | ((dest) << 8));                  \
        shift -= 2;                                                           \
      } else {                                                                \
        sh4_emit_word(sh4_opcode_shll1_reg | ((dest) << 8));                  \
        shift -= 1;                                                           \
      }                                                                       \
    }                                                                         \
  }

#define generate_shift_left(ireg, imm)                                        \
  SH4_LOG(DBG_INFO, "SHL: emitting SHL r%d, #0x%04x\n", ireg, imm); \
  sh4_emit_shl_reg_imm(ireg, imm)                                       \

#define generate_shift_right(ireg, imm)                                       \
    sh4_emit_shr_reg_imm(ireg, imm)                                       \

#define generate_rotate_right(ireg, imm)                                      \
  sh4_emit_ror_reg_imm(ireg, imm)                                             \

// New: Shift left with fixed amounts
// #define sh4_emit_shl_reg_imm(dest, imm)                                       \
// sh4_emit_opcode_1b_ext_reg(shl_reg_imm, dest);                              \
// sh4_emit_byte(imm)                                                          \

// New: Shift right logical with fixed amounts
#define sh4_emit_shr_reg_imm(dest, imm)                                       \
  {                                                                           \
    u32 shift = (imm);                                                        \
    while (shift > 0) {                                                       \
      if (shift >= 16) {                                                      \
        sh4_emit_word(sh4_opcode_shlr16_reg | ((dest) << 8));                 \
        shift -= 16;                                                          \
      } else if (shift >= 8) {                                                \
        sh4_emit_word(sh4_opcode_shlr8_reg | ((dest) << 8));                  \
        shift -= 8;                                                           \
      } else if (shift >= 2) {                                                \
        sh4_emit_word(sh4_opcode_shlr2_reg | ((dest) << 8));                  \
        shift -= 2;                                                           \
      } else {                                                                \
        sh4_emit_word(sh4_opcode_shlr1_reg | ((dest) << 8));                  \
        shift -= 1;                                                           \
      }                                                                       \
    }                                                                         \
  }

#define generate_add(ireg_dest, ireg_src)                                     \
  sh4_emit_add_reg_reg(ireg_dest, ireg_src)                      \

#define generate_sub(ireg_dest, ireg_src)                                     \
  sh4_emit_sub_reg_reg(ireg_dest, ireg_src)                       \

#define generate_or(ireg_dest, ireg_src)                                      \
  sh4_emit_or_reg_reg(ireg_dest, ireg_src)                        \

#define generate_xor(ireg_dest, ireg_src)                                     \
  sh4_emit_xor_reg_reg(ireg_dest, ireg_src)                       \

#define generate_add_imm(ireg, imm)                                           \
  sh4_emit_add_reg_imm(ireg, imm)                                       \

#define generate_sub_imm(ireg, imm)                                           \
  sh4_emit_word(0x7000 | ((ireg) << 8) | (-(imm)))         \

#define generate_xor_imm(ireg, imm)                                           \
  sh4_emit_xor_reg_imm(ireg, imm)                                       \

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm)                    \
  sh4_emit_lea_reg_mem(ireg_dest, ireg_src, imm)                  \

#define generate_and_imm(ireg, imm)                                           \
  SH4_LOG(DBG_INFO, "AND: emitting AND r%d, #0x%04x\n", ireg, imm); \
sh4_emit_and_reg_imm(reg_##ireg, imm)                                       \

  typedef enum {
    no_flags = 0,
    flags = 1
  } flags_operation_type;

#define sh4_emit_and_reg_imm(dest, imm)                                       \
  if ((imm) <= 0xFF && (imm) >= 0 && (dest) == sh4_reg_number_r0) {           \
    sh4_emit_word(0xC900 | ((imm) & 0xFF));                                  \
  } else {                                                                    \
    sh4_emit_mov_reg_imm(reg_a1, imm);                                        \
    sh4_emit_and_reg_reg(dest, reg_a1);                                       \
  }

#define sh4_emit_and_reg_reg(dest, source) \
  sh4_emit_word(sh4_opcode_and_reg_reg | ((dest) << 8) | ((source) << 4)) \

#define sh4_emit_tst_reg_reg(rn, rm) \
  SH4_LOG(DBG_INFO, "TST: emitting TST r%d, r%d\n", rn, rm); \
  sh4_emit_word(sh4_opcode_tst_reg_reg | ((rn) << 8) | ((rm) << 4)) \

#define generate_and(ireg_dest, ireg_src)                                     \
  sh4_emit_and_reg_reg(ireg_dest, ireg_src)                      \

#define generate_mov(ireg_dest, ireg_src) \
  SH4_LOG(DBG_INFO, "generate_mov: MOV r%d -> r%d\n", ireg_src, ireg_dest); \
  sh4_emit_mov_reg_reg(ireg_dest, ireg_src)

#define generate_multiply(ireg) \
  sh4_emit_mov_reg_reg(reg_a4, ireg); \
  generate_function_call(execute_mul); \
  sh4_emit_mov_reg_reg(reg_rv, reg_a0)  \

  #define generate_multiply_u64(ireg) \
  sh4_emit_mov_reg_reg(a4, ireg); \
  generate_function_call(execute_mul_long_u64); \
  sh4_emit_mov_reg_reg(a0, reg_rv_lo); \
  sh4_emit_mov_reg_reg(a1, reg_rv_hi)

#define generate_multiply_s64(ireg) \
  sh4_emit_mov_reg_reg(a4, ireg); \
  generate_function_call(execute_mul_long_s64); \
  sh4_emit_mov_reg_reg(a0, reg_rv_lo); \
  sh4_emit_mov_reg_reg(a1, reg_rv_hi)

#define generate_multiply_s64_add(ireg_src, ireg_lo, ireg_hi)                 \
  sh4_emit_mov_reg_reg(a4, ireg_src);                                         \
  sh4_emit_mov_reg_reg(a5, ireg_lo);                                          \
  sh4_emit_mov_reg_reg(a6, ireg_hi);                                          \
  generate_function_call(execute_mul_long_add_s64);                           \
  sh4_emit_mov_reg_reg(a0, reg_rv_lo);                                        \
  sh4_emit_mov_reg_reg(a1, reg_rv_hi)


#define generate_multiply_u64_add(ireg_src, ireg_lo, ireg_hi)                 \
  sh4_emit_mov_reg_reg(a4, ireg_src);                                         \
  sh4_emit_mov_reg_reg(a5, ireg_lo);                                          \
  sh4_emit_mov_reg_reg(a6, ireg_hi);                                          \
  generate_function_call(execute_mul_long_add_u64);                           \
  sh4_emit_mov_reg_reg(a0, reg_rv_lo);                                        \
  sh4_emit_mov_reg_reg(a1, reg_rv_hi)


#define generate_function_call(func) \
  do { \
      u32 func_addr = (u32)(func); \
      SH4_LOG(DBG_INFO, "Generating call to %s at %08x\n", #func, func_addr); \
      /* Load function address into R0 via literal pool */ \
      sh4_emit_mov_reg_imm(a0, func_addr); \
      /* Emit JSR @R0 */ \
      sh4_emit_word(0x400B | (a0 << 8)); /* JSR @R0 */ \
      sh4_emit_word(sh4_opcode_nop); /* NOP */ \
  } while (0)

  
#define generate_branch_filler_true(ireg_dest, ireg_src, writeback_location) \
  SH4_LOG(DBG_INFO, "generate_branch_filler_true: ireg_dest=%d, ireg_src=%d, writeback_location=%p\n", ireg_dest, ireg_src, (void*)writeback_location); \
  sh4_emit_test_reg_imm(ireg_dest, 1); \
  sh4_emit_j_filler(sh4_condition_z, writeback_location) \

#define generate_branch_filler_false(ireg_dest, ireg_src, writeback_location) \
  SH4_LOG(DBG_INFO, "generate_branch_filler_false: ireg_dest=%d, ireg_src=%d, writeback_location=%p\n", ireg_dest, ireg_src, (void*)writeback_location); \
  sh4_emit_test_reg_imm(ireg_dest, 1);                                  \
  sh4_emit_j_filler(sh4_condition_nz, writeback_location)                \

#define generate_branch_filler_equal(ireg_dest, ireg_src, writeback_location) \
  SH4_LOG(DBG_INFO, "generate_branch_filler_equal: ireg_dest=%d, ireg_src=%d, writeback_location=%p\n", ireg_dest, ireg_src, (void*)writeback_location); \
  sh4_emit_cmp_reg_reg(ireg_dest, ireg_src);                      \
  sh4_emit_j_filler(sh4_condition_nz, writeback_location)                \

#define generate_branch_filler_not_equal(ireg_dest, ireg_src,  writeback_location)\
  SH4_LOG(DBG_INFO, "generate_branch_filler_not_equal: ireg_dest=%d, ireg_src=%d, writeback_location=%p\n", ireg_dest, ireg_src, (void*)writeback_location); \
  sh4_emit_cmp_reg_reg(ireg_dest, ireg_src);                      \
  sh4_emit_j_filler(sh4_condition_z, writeback_location)                 \

#define generate_update_pc_reg()                                              \
  SH4_LOG(DBG_INFO, "generate_update_pc_reg: Updating PC to R%d\n", a0); \
  generate_update_pc(pc);                                                     \
  generate_store_reg(a0, REG_PC)                                              \

#define generate_update_pc(new_pc) \
do { \
  SH4_LOG(DBG_INFO, "generate_update_pc: Updating PC to %08x\n", new_pc); \
  sh4_emit_mov_reg_imm(a4, new_pc); \
} while (0)

#define generate_cycle_update() \
do { \
  SH4_LOG(DBG_INFO, "Cycle update: %d\n", cycle_count); \
        sh4_emit_sub_reg_imm(reg_cycles, cycle_count); \
        cycle_count = 0; \
} while (0)

#define generate_branch_patch_conditional(dest, target) \
do { \
    if (!(dest)) { \
        SH4_LOG(DBG_INFO, "generate_branch_patch_conditional: [PATCH ERROR] Conditional dest is NULL for target=%p!\n", (void*)target); \
        break; \
    } \
    SH4_LOG(DBG_INFO, "generate_branch_patch_conditional: %p -> %p\n", (void*)dest, (void*)target); \
    volatile u32 src = (u32)(uintptr_t)(dest); \
    volatile u32 dst = (u32)(uintptr_t)(target) - block_prologue_size; /* Adjust for prologue */ \
    s32 displacement = ((s32)(dst - (src + 4))) >> 1; \
    SH4_LOG(DBG_INFO, "generate_branch_patch_conditional: Branch patch conditional: %p -> %p (disp=%d)\n", (void*)src, (void*)dst, displacement); \
    if (displacement > 127 || displacement < -128) { \
        SH4_LOG(DBG_INFO, "generate_branch_patch_conditional: [PATCH] Using indirect jump for large displacement: %p -> %p (disp=%d)\n", (void*)src, (void*)dst, displacement); \
        u16* movl_addr = (u16*)(dest - 2); \
        u32 movl_addr_u32 = (u32)(uintptr_t)movl_addr; \
        u32 literal_addr = (u32)(uintptr_t)translation_ptr; \
        s32 disp = ((literal_addr + 4 - movl_addr_u32 - 4) >> 1); \
        if ((disp & 0xFFFFFF00) != 0 && (disp & 0xFFFFFF00) != 0xFFFFFF00) { \
            SH4_LOG(DBG_ERROR, "generate_branch_patch_conditional: Indirect jump displacement out of range: %p -> %p (disp=%d)\n", \
                    movl_addr, translation_ptr, disp); \
            break; \
        } \
        *movl_addr = 0xD000 | (disp & 0xFF); \
        *(u16*)dest = 0x402B; \
        *(u16*)((u8*)dest + 2) = sh4_opcode_nop; \
        *(u32*)translation_ptr = dst; \
        translation_ptr += 4; \
        SH4_LOG(DBG_INFO, "generate_branch_patch_conditional: Patched MOV.L @%p = %04x (disp=%d), value=%08x\n", \
                movl_addr, *movl_addr, disp, dst); \
        break; \
    } \
    *((u16 *)(dest)) = (u16)((sh4_opcode_bf & 0xFF00) | (displacement & 0x00FF)); \
    asm volatile("" ::: "memory"); \
} while (0)

#define generate_branch_patch_unconditional(dest, target) \
do { \
    if (!(dest)) { \
        SH4_LOG(DBG_INFO, "generate_branch_patch_unconditional: [PATCH ERROR] Unconditional dest is NULL for target=%p!\n", (void*)target); \
        break; \
    } \
    SH4_LOG(DBG_INFO, "generate_branch_patch_unconditional: %p -> %p\n", (void*)dest, (void*)target); \
    volatile u32 src = (u32)(uintptr_t)(dest); \
    volatile u32 dst = (u32)(uintptr_t)(target) - block_prologue_size; /* Adjust for prologue */ \
    s32 displacement = ((s32)(dst - (src + 4))) >> 1; \
    SH4_LOG(DBG_INFO, "generate_branch_patch_unconditional: %p -> %p (disp=%d)\n", (void*)src, (void*)dst, displacement); \
    if (displacement < -2048 || displacement > 2047) { \
        SH4_LOG(DBG_INFO, "generate_branch_patch_unconditional: [PATCH] Using indirect jump for large displacement: %p -> %p (disp=%d)\n", (void*)src, (void*)dst, displacement); \
        u16* movl_addr = (u16*)(dest - 2); \
        u32 movl_addr_u32 = (u32)(uintptr_t)movl_addr; \
        u32 literal_addr = (u32)(uintptr_t)translation_ptr; \
        s32 disp = ((literal_addr + 4 - movl_addr_u32 - 4) >> 1); \
        if ((disp & 0xFFFFFF00) != 0 && (disp & 0xFFFFFF00) != 0xFFFFFF00) { \
            SH4_LOG(DBG_ERROR, "generate_branch_patch_unconditional: Indirect jump displacement out of range: %p -> %p (disp=%d)\n", \
                    movl_addr, translation_ptr, disp); \
            break; \
        } \
        *movl_addr = 0xD000 | (disp & 0xFF); \
        *(u16*)dest = 0x402B; \
        *(u16*)((u8*)dest + 2) = sh4_opcode_nop; \
        *(u32*)translation_ptr = dst; \
        translation_ptr += 4; \
        SH4_LOG(DBG_INFO, "generate_branch_patch_unconditional: Patched MOV.L @%p = %04x (disp=%d), value=%08x\n", \
                movl_addr, *movl_addr, disp, dst); \
        break; \
    } \
    *((u16 *)(dest)) = (u16)((sh4_opcode_bra & 0xF000) | (displacement & 0x0FFF)); \
    asm volatile("" ::: "memory"); \
} while (0)

#define generate_branch_no_cycle_update(writeback_location, new_pc) \
do { \
    SH4_LOG(DBG_INFO, "Branch no cycle update: new_pc=%08x, source=%p\n", new_pc, writeback_location); \
    sh4_emit_mov_reg_imm(a4, new_pc); \
    sh4_emit_mov_mem_reg(a4, reg_base, REG_PC); \
    sh4_emit_cmp_pl(reg_cycles); \
    u16* skip_update = (u16*)translation_ptr; \
    sh4_emit_word(sh4_opcode_bt | 0x00); \
    sh4_emit_word(sh4_opcode_nop); \
    generate_function_call(sh4_update_gba); \
    sh4_emit_jmp_filler(writeback_location); \
} while (0)
  
#define generate_branch_cycle_update(writeback_location, new_pc) \
do { \
    generate_cycle_update(); \
    generate_branch_no_cycle_update(writeback_location, new_pc); \
} while (0)
  
#define generate_branch() \
do { \
    SH4_LOG(DBG_INFO, "generate_branch: target=%08x, source=%p\n", \
            block_exits[block_exit_position].branch_target, block_exits[block_exit_position].branch_source); \
    if (condition == 0x0E) { \
        generate_branch_cycle_update( \
            block_exits[block_exit_position].branch_source, \
            block_exits[block_exit_position].branch_target); \
    } else { \
        generate_branch_no_cycle_update( \
            block_exits[block_exit_position].branch_source, \
            block_exits[block_exit_position].branch_target); \
    } \
    block_exit_position++; \
} while (0)


  #define generate_conditional_branch(ireg_a, ireg_b, cond, writeback_location) \
  do { \
    SH4_LOG(DBG_INFO, "[TRANSLATE] Conditional branch: type=%d, a=R%d, b=R%d, PC=%08x\n", \
            cond, ireg_a, ireg_b, reg[REG_PC]); \
    switch (cond) { \
      case COND_TRUE:       generate_branch_filler_true(ireg_a, ireg_b, writeback_location); break; \
      case COND_FALSE:      generate_branch_filler_false(ireg_a, ireg_b, writeback_location); break; \
      case COND_EQUAL:      generate_branch_filler_equal(ireg_a, ireg_b, writeback_location); break; \
      case COND_NOT_EQUAL:  generate_branch_filler_not_equal(ireg_a, ireg_b, writeback_location); break; \
    } \
  } while (0)

  #define generate_indirect_branch_cycle_update(type) \
  SH4_LOG(DBG_INFO, "generate_indirect_branch_cycle_update:\n"); \
  generate_cycle_update(); \
  generate_function_call(sh4_indirect_branch_##type); \
  block_exit_position = 0; \

#define generate_indirect_branch_no_cycle_update(type) \
  SH4_LOG(DBG_INFO, "generate_indirect_branch_no_cycle_update:\n"); \
  generate_function_call(sh4_indirect_branch_##type); \
  block_exit_position=0; \


  

  #define block_prologue_size 8

  #define generate_block_prologue() \
  do { \
      SH4_LOG(DBG_INFO, "Generating block prologue at %p\n", translation_ptr); \
      literal_count = 0; \
      cycle_count = 0; \
      sh4_emit_mov_reg_imm(reg_base, (u32)reg); \
      sh4_emit_mov_reg_imm(a0, (u32)log_block_start); \
      sh4_emit_word(0x400B | (a0 << 8)); \
      sh4_emit_word(0x0009); \
  } while (0)

    #define finalize_literal_pool() \
    do { \
        SH4_LOG(DBG_INFO, "Literals before finalization: count=%d\n", literal_count); \
        translation_ptr = (u8*)(((uintptr_t)translation_ptr + 3) & ~3); \
        u8* literal_start = translation_ptr; \
        for (u32 i = 0; i < literal_count; i++) { \
            u32 literal_addr = (u32)translation_ptr; \
            SH4_LOG(DBG_INFO, "Literal[%d]: Writing value=%08x at addr=%08x\n", \
                    i, literals[i].immediate, literal_addr); \
            *((u32*)translation_ptr) = literals[i].immediate; \
            translation_ptr += 4; \
            u32 movl_pc = (u32)literals[i].movl_addr; \
            u32 movl_pc_aligned = ((movl_pc & ~3) + 4);\
            u32 disp = (literal_addr - movl_pc_aligned) >> 2; \
            if (disp > 0xFF) { \
                SH4_LOG(DBG_ERROR, "Literal disp too large at %p (disp=%u)\n", literals[i].movl_addr, disp); \
                reg[CPU_HALT_STATE] = 1; \
                break; \
            } \
            *literals[i].movl_addr = 0xD000 | (literals[i].destination << 8) | (disp & 0xFF); \
            SH4_LOG(DBG_INFO, "Patched MOV.L @%p = %04x (disp=%d)\n", \
                    literals[i].movl_addr, *literals[i].movl_addr, disp); \
        } \
        *((u16*)translation_ptr) = 0xFFFF; \
        translation_ptr += 2; \
        *((u16*)translation_ptr) = 0xFFFF; \
        translation_ptr += 2; \
        literal_count = 0; \
    } while (0)
    
    #define generate_exit_block() \
    do { \
        SH4_LOG(DBG_INFO, "Exiting block at %p\n", translation_ptr); \
        finalize_literal_pool(); \
        u8* block_start = NULL; \
        u32 hash_target = ((pc * 2654435761U) >> 16) & (ROM_BRANCH_HASH_SIZE - 1); \
        u32* block_ptr = rom_branch_hash[hash_target]; \
        while (block_ptr) { \
            if (block_ptr[0] == pc) { \
                block_start = (u8*)(block_ptr + 2); \
                break; \
            } \
            block_ptr = (u32*)block_ptr[1]; \
        } \
        if (block_start) { \
            SH4_LOG(DBG_INFO, "Flushing block: start=%p, size=%u\n", block_start, translation_ptr - block_start + 0x80); \
            icache_flush_range((void*)block_start, (translation_ptr - block_start) + 0x80); \
            dcache_flush_range((void*)block_start, (translation_ptr - block_start) + 0x80); \
        } else { \
            SH4_LOG(DBG_WARNING, "No block start found for PC=%08x\n", pc); \
        } \
    } while (0)

#define generate_block_extra_vars_arm()                                        \
  void generate_indirect_branch_arm()                                          \
  {                                                                            \
    SH4_LOG(DBG_INFO, "generate_block_extra_vars_arm: generate_indirect_branch_arm: condition=%02x\n", condition); \
    if (condition == 0x0E)                                                     \
    {                                                                          \
      generate_indirect_branch_cycle_update(arm);                              \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      generate_indirect_branch_no_cycle_update(arm);                           \
    }                                                                          \
  }                                                                            \
                                                                               \
  void generate_indirect_branch_dual()                                         \
  {                                                                            \
    if (condition == 0x0E)                                                     \
    {                                                                          \
      generate_indirect_branch_cycle_update(dual);                             \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      generate_indirect_branch_no_cycle_update(dual);                          \
    }                                                                          \
  }                                                                            \


#define generate_block_extra_vars_thumb()                                     \


#define translate_invalidate_dcache() \
do { \
    SH4_LOG(DBG_INFO, "Invalidate Cache at PC=%08x, skipping full flush\n", pc); \
    /* Targeted flush only if necessary, e.g., for self-modifying code */ \
    /* dcache_flush_range((uint32)rom_translation_cache, 0x1000); */ \
    /* icache_flush_range((uint32)rom_translation_cache, 0x1000); */ \
} while (0)

#define calculate_z_flag(dest)                                                \
  reg[REG_Z_FLAG] = (dest == 0);                                               \

#define calculate_n_flag(dest)                                                \
  reg[REG_N_FLAG] = ((signed)dest < 0);                                        \

#define calculate_c_flag_sub(dest, src_a, src_b)                              \
  reg[REG_C_FLAG] = ((unsigned)src_b <= (unsigned)src_a);                      \

#define calculate_v_flag_sub(dest, src_a, src_b)                              \
  reg[REG_V_FLAG] = ((signed)src_b > (signed)src_a) != ((signed)dest < 0);     \

#define calculate_c_flag_add(dest, src_a, src_b)                              \
  reg[REG_C_FLAG] = ((unsigned)dest < (unsigned)src_a);                        \

#define calculate_v_flag_add(dest, src_a, src_b)                              \
  reg[REG_V_FLAG] = ((signed)dest < (signed)src_a) != ((signed)src_b < 0);     \

#define get_shift_imm()                                                       \
  u32 shift = (opcode >> 7) & 0x1F;                                            \

#define generate_shift_reg(ireg, name, flags_op)                              \
  generate_load_reg_pc(ireg, rm, 12);                                         \
  generate_load_reg(a4, ((opcode >> 8) & 0x0F));                              \
  generate_function_call(execute_##name##_##flags_op##_reg);                  \
  generate_mov(ireg, rv);                                                      \

  
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

#define generate_shift_imm(ireg, name, flags_op)                              \
    SH4_LOG(DBG_INFO, "generate_shift_imm: ireg=%d, name=%s, flags_op=%s\n", ireg, #name, #flags_op); \
    get_shift_imm();                                                            \
    generate_shift_imm_##name##_##flags_op(ireg)                                \

#define generate_shift_imm_lsl_no_flags(ireg)                                 \
    SH4_LOG(DBG_INFO, "generate_shift_imm_lsl_no_flags: ireg=%d\n", ireg); \
    generate_load_reg_pc(ireg, rm, 8);                                          \
    if(shift != 0)                                                              \
    {                                                                           \
      SH4_LOG(DBG_INFO, "generate_shift_imm_lsl_no_flags: shift=%d\n", shift); \
      generate_shift_left(ireg, shift);                                         \
    }                                                                           \

#define generate_shift_imm_lsr_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_shift_right(ireg, shift);                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_no_flags(ireg)                                 \
    generate_load_reg_pc(ireg, rm, 8);                                          \
    if(shift != 0)                                                              \
    {                                                                           \
      generate_shift_right_arithmetic(ireg, shift);                             \
    }                                                                           \
    else                                                                        \
    {                                                                           \
      generate_shift_right_arithmetic(ireg, 31);                                \
    }                                                                           \

#define generate_shift_imm_lsl_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right(a1, (32 - shift));                                   \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_left(ireg, shift);                                         \
  }                                                                           \

#define generate_shift_imm_lsl_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right(a1, (32 - shift));                                   \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_left(ireg, shift);                                         \
  }                                                                           \

#define generate_shift_imm_lsr_flags(ireg)                                    \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right(a1, shift - 1);                                      \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_right(ireg, shift);                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a1, rm, 8);                                          \
    generate_shift_right(a1, 31);                                             \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_load_imm(ireg, 0);                                               \
  }                                                                           \

#define generate_shift_imm_asr_flags(ireg)                                    \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right_arithmetic(a1, shift - 1);                           \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_shift_right_arithmetic(ireg, shift);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a0, rm, 8);                                          \
    generate_shift_right_arithmetic(ireg, 31);                                \
    generate_mov(a1, ireg);                                                   \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
  }                                                                           \

#define generate_shift_imm_ror_flags(ireg)                                    \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_mov(a1, ireg);                                                   \
    generate_shift_right_arithmetic(a1, shift - 1);                           \
    generate_and_imm(a1, 1);                                                  \
    generate_store_reg(a1, REG_C_FLAG);                                       \
    generate_rotate_right(ireg, shift);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_function_call(execute_rrx_flags);                                \
    generate_mov(ireg, rv);                                                   \
  }                                                                           \



  #define generate_load_rm_sh(flags_op)                                         \
  SH4_LOG(DBG_INFO, "generate_load_rm_sh:(opcode >> 4) & 0x07 = %d, flags_op=%s\n", (opcode >> 4) & 0x07, #flags_op);                                      \
  switch((opcode >> 4) & 0x07)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a4, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSL reg */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_reg(a4, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a4, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR reg */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_reg(a4, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x4:                                                                 \
    {                                                                         \
      generate_shift_imm(a4, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR reg */                                                             \
    case 0x5:                                                                 \
    {                                                                         \
      generate_shift_reg(a4, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x6:                                                                 \
    {                                                                         \
      generate_shift_imm(a4, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR reg */                                                             \
    case 0x7:                                                                 \
    {                                                                         \
      generate_shift_reg(a4, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define generate_load_offset_sh()                                             \
  switch((opcode >> 5) & 0x03)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsl, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, lsr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, asr, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_imm(a1, ror, no_flags);                                  \
      break;                                                                  \
    }                                                                         \
  }                                                                           \

#define generate_shift_imm_ror_no_flags(ireg)                                 \
  if(shift != 0)                                                              \
  {                                                                           \
    generate_load_reg_pc(ireg, rm, 8);                                        \
    generate_rotate_right(ireg, shift);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg_pc(a0, rm, 8);                                          \
    generate_function_call(execute_rrx);                                      \
    generate_mov(ireg, rv);                                                   \
  }                                                                           \

#define calculate_flags_add(dest, src_a, src_b)                               \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  calculate_c_flag_add(dest, src_a, src_b);                                   \
  calculate_v_flag_add(dest, src_a, src_b)                                    \


#define calculate_flags_sub(dest, src_a, src_b)                               \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  calculate_c_flag_sub(dest, src_a, src_b);                                   \
  calculate_v_flag_sub(dest, src_a, src_b)                                    \

#define calculate_flags_logic(dest)                                           \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest)                                                      \

#define extract_flags()                                                       \
  reg[REG_N_FLAG] = reg[REG_CPSR] >> 31;                                      \
  reg[REG_Z_FLAG] = (reg[REG_CPSR] >> 30) & 0x01;                             \
  reg[REG_C_FLAG] = (reg[REG_CPSR] >> 29) & 0x01;                             \
  reg[REG_V_FLAG] = (reg[REG_CPSR] >> 28) & 0x01;                             \

#define collapse_flags()                                                      \
  reg[REG_CPSR] = (reg[REG_N_FLAG] << 31) | (reg[REG_Z_FLAG] << 30) |         \
   (reg[REG_C_FLAG] << 29) | (reg[REG_V_FLAG] << 28) |                        \
   reg[REG_CPSR] & 0xFF                                                       \

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.

#define check_for_interrupts()                                                \
  if((io_registers[REG_IE] & io_registers[REG_IF]) &&                         \
   io_registers[REG_IME] && ((reg[REG_CPSR] & 0x80) == 0))                    \
  {                                                                           \
    reg_mode[MODE_IRQ][6] = reg[REG_PC] + 4;                                  \
    spsr[MODE_IRQ] = reg[REG_CPSR];                                           \
    reg[REG_CPSR] = 0xD2;                                                     \
    address = 0x00000018;                                                     \
    set_cpu_mode(MODE_IRQ);                                                   \
  }                                                                           \

#define generate_load_reg_pc(ireg, reg_index, pc_offset)                      \
  if(reg_index == 15)                                                         \
  {                                                                           \
    SH4_LOG(DBG_INFO, "Loading PC with offset %d\n", pc_offset);             \
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

  #define generate_store_reg_pc_no_flags(ireg, reg_index)                       \
  SH4_LOG(DBG_INFO, "generate_store_reg_pc_no_flags: generate_store_reg(ireg=%d, reg_index=%d)\n", ireg, reg_index); \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    SH4_LOG(DBG_INFO, "generate_store_reg_pc_no_flags: reg_index == 15, generate_mov(a4, ireg=%d)\n", ireg); \
    generate_mov(a4, ireg);                                                   \
    generate_indirect_branch_arm();                                           \
  }                                                                           \
  
  


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

#define generate_store_reg_pc_flags(ireg, reg_index)                          \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a4, ireg);                                                   \
    generate_function_call(execute_spsr_restore);                             \
    generate_mov(a0, rv);                                                     \
    generate_indirect_branch_dual();                                          \
  }                                                                           \

typedef enum
{
  CONDITION_TRUE,
  CONDITION_FALSE,
  CONDITION_EQUAL,
  CONDITION_NOT_EQUAL
} condition_check_type;


#define generate_condition_eq(ireg_a, ireg_b) \
do { \
    generate_load_reg(ireg_a, REG_Z_FLAG); \
    condition_check = (reg[REG_Z_FLAG] == 1) ? CONDITION_TRUE : CONDITION_FALSE; \
    SH4_LOG(DBG_INFO, "Condition EQ: Z_FLAG=%d, condition_check=%d\n", reg[REG_Z_FLAG], condition_check); \
} while (0)

  #define generate_condition_ne(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_Z_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

  #define generate_condition_cs(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

  #define generate_condition_cc(ireg_a, ireg_b) \
  do { \
      generate_load_reg(ireg_a, REG_C_FLAG); \
      condition_check = (reg[REG_C_FLAG] == 1) ? CONDITION_TRUE : CONDITION_FALSE; \
      SH4_LOG(DBG_INFO, "Condition CC: C_FLAG=%d, condition_check=%d\n", reg[REG_C_FLAG], condition_check); \
  } while (0)
  

#define generate_condition_mi(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

  #define generate_condition_pl(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_vs(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_V_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_vc(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_V_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

  #define generate_condition_hi(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  generate_xor_imm(ireg_a, 1);                                                \
  generate_load_reg(ireg_b, REG_Z_FLAG);                                      \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_ls(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  generate_xor_imm(ireg_a, 1);                                                \
  generate_load_reg(ireg_b, REG_Z_FLAG);                                      \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_ge(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  condition_check = CONDITION_EQUAL                                           \

  #define generate_condition_lt(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  condition_check = CONDITION_NOT_EQUAL                                       \

#define generate_condition_gt(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  generate_xor(ireg_b, ireg_a);                                               \
  generate_load_reg(a4, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_le(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  generate_xor(ireg_b, ireg_a);                                               \
  generate_load_reg(a4, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_TRUE                                            \

  #define generate_condition(ireg_a, ireg_b)                                    \
  SH4_LOG(DBG_INFO, "generate_condition: condition=%02x, ireg_a=%d, ireg_b=%d\n", condition, ireg_a, ireg_b); \
  switch(condition)                                                           \
  {                                                                           \
    case 0x0:                                                                 \
      generate_condition_eq(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x1:                                                                 \
      generate_condition_ne(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x2:                                                                 \
      generate_condition_cs(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x3:                                                                 \
      generate_condition_cc(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x4:                                                                 \
      generate_condition_mi(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x5:                                                                 \
      generate_condition_pl(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x6:                                                                 \
      generate_condition_vs(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x7:                                                                 \
      generate_condition_vc(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x8:                                                                 \
      generate_condition_hi(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0x9:                                                                 \
      generate_condition_ls(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xA:                                                                 \
      generate_condition_ge(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xB:                                                                 \
      generate_condition_lt(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xC:                                                                 \
      generate_condition_gt(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xD:                                                                 \
      generate_condition_le(ireg_a, ireg_b);                                  \
      break;                                                                  \
                                                                              \
    case 0xE:                                                                 \
      /* AL       */                                                          \
      break;                                                                  \
                                                                              \
    case 0xF:                                                                 \
      /* Reserved */                                                          \
      break;                                                                  \
}                                                                             \

#define generate_conditional_branch_type(ireg_a, ireg_b) \
  do { \
    SH4_LOG(DBG_INFO, "generate_conditional_branch_type: condition_check=%d\n", condition_check); \
    switch (condition_check) { \
      case CONDITION_TRUE: \
        generate_branch_filler_true(ireg_a, ireg_b, backpatch_address); \
        break; \
      case CONDITION_FALSE: \
        generate_branch_filler_false(ireg_a, ireg_b, backpatch_address); \
        break; \
      case CONDITION_EQUAL: \
        generate_branch_filler_equal(ireg_a, ireg_b, backpatch_address); \
        break; \
      case CONDITION_NOT_EQUAL: \
        generate_branch_filler_not_equal(ireg_a, ireg_b, backpatch_address); \
        break; \
    } \
  } while (0)


// #define rm_op_reg rm
// #define rm_op_imm imm

#define arm_data_proc_reg_flags()                                             \
  arm_decode_data_proc_reg();                                                 \
  if(flag_status & 0x02)                                                      \
  {                                                                           \
    generate_load_rm_sh(flags)                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_rm_sh(no_flags);                                            \
  }                                                                           \

#define arm_data_proc_reg()                                                   \
  arm_decode_data_proc_reg();                                                 \
  SH4_LOG(DBG_INFO, "arm_data_proc_reg: rm=%d, rn=%d, rd=%d\n", rm, rn, rd); \
  generate_load_rm_sh(no_flags)                                               \

#define arm_data_proc_imm()                                                   \
  arm_decode_data_proc_imm();                                                 \
  SH4_LOG(DBG_INFO, "arm_data_proc_imm: imm=%08x\n", imm); \
  generate_load_imm(a4, imm)                                                  \


  #define arm_data_proc(name, type, flags_op)                                   \
  {                                                                           \
    SH4_LOG(DBG_INFO, "arm_data_proc: name %s,type =%s, flag_op=%s\n", #name, #type,#flags_op);                       \
    arm_data_proc_##type();                                                  \
    sh4_emit_mov_reg_reg(a4, a0); /* rm into a4 */ \
    generate_load_reg_pc(a5, rn, 8);                                          \
    generate_function_call(execute_##name);                                   \
    generate_store_reg_pc_##flags_op(rv, rd);                                 \
  }                                                                           \

  #define arm_data_proc_test(name, type) \
  { \
    SH4_LOG(DBG_INFO, "arm_data_proc_test: name %s,type =%s\n", #name, #type);                       \    
    arm_data_proc_##type(); \
    sh4_emit_mov_reg_reg(a4, a0); /* rm */ \
    generate_load_reg_pc(a5, rn, 8); \
    generate_function_call(execute_##name); \
  }                                                                       \

  #define arm_data_proc_unary(name, type, flags_op) \
  do { \
      SH4_LOG(DBG_INFO, "arm_data_proc_unary: %s, type=%s, flags_op=%s, opcode=%08x, pc=%08x\n", \
              #name, #type, #flags_op, opcode, pc); \
      arm_data_proc_##type(); /* Loads operand into R4 */ \
      generate_function_call(execute_##name); /* execute_##name(R4) -> R0 */ \
      sh4_emit_mov_reg_reg(a4, rv); /* MOV R0, R4 to preserve result */ \
      generate_store_reg_pc_##flags_op(a4, rd); /* Store R4 to reg[rd] */ \
      SH4_LOG(DBG_INFO, "arm_data_proc_unary: Emitted store R4 to reg[%d]\n", rd); \
  } while (0)

#define arm_data_proc_mov(type)                                               \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_store_reg_pc_no_flags(a4, rd);                                     \
}                                                                             \

static void function_cc execute_mul_flags(u32 dest)
{
  calculate_z_flag(dest);
  calculate_n_flag(dest);
}

#define arm_multiply_flags_yes()                                              \
  generate_function_call(execute_mul_flags)                                   \

#define arm_multiply_flags_no(_dest)                                          \

#define arm_multiply_add_no()                                                 \

#define arm_multiply_add_yes() \
  generate_load_reg(a5, rn);               /* Load rn into a5 */ \
  generate_add(a4, a5)                     /* Add a5 to a4 (result stays in a4) */

  #define arm_multiply(add_op, flags) \
  { \
    arm_decode_multiply(); \
    generate_load_reg(a4, rm);      /* rm → a4 */ \
    generate_load_reg(a5, rs);      /* rs → a5 */ \
    generate_function_call(execute_mul); \
    sh4_emit_mov_reg_reg(a4, rv);   /* result (r0) → a4 */ \
    arm_multiply_add_##add_op(); \
    generate_store_reg(a4, rd);     /* store result in rd */ \
    arm_multiply_flags_##flags(); \
  }
  

  static void function_cc execute_mul_long_flags(u32 dest_lo, u32 dest_hi)
  {
    reg[REG_Z_FLAG] = (dest_lo == 0) & (dest_hi == 0);
    calculate_n_flag(dest_hi);
  }

  #define arm_multiply_long_flags_yes()                                      \
  sh4_emit_mov_reg_imm(a4, reg_rv_lo); /* Load result lo → a4 */             \
  sh4_emit_mov_reg_imm(a5, reg_rv_hi); /* Load result hi → a5 */             \
  generate_function_call(execute_mul_long_flags)



#define arm_multiply_long_flags_no(_dest)                                     \

#define arm_multiply_long_add_yes(name) \
    generate_load_reg(a5, rdlo); \
    generate_load_reg(s0, rdhi); \
    generate_multiply_##name(a1, a5, s0)

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name(a1);                                                \

  #define arm_multiply_long(name, add_op, flags)                            \
  {                                                                         \
      arm_decode_multiply_long();                                          \
      generate_load_reg(a4, rm);                                           \
      generate_load_reg(a5, rs);                                           \
      arm_multiply_long_add_##add_op(name);                                \
      sh4_emit_mov_reg_imm(a0, reg_rv_lo);                                   \
      sh4_emit_mov_reg_imm(a1, reg_rv_hi);                                   \
      generate_store_reg(a0, rdlo);                                        \
      generate_store_reg(a1, rdhi);                                        \
      arm_multiply_long_flags_##flags();                                   \
  }                                                                        \

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

u32 function_cc execute_store_cpsr_body(u32 _cpsr, u32 store_mask, u32 address)
{
  SH4_LOG(DBG_INFO, "execute_store_cpsr_body entry: _cpsr=%08x, store_mask=%08x, address=%08x\n", _cpsr, store_mask, address);
  reg[REG_CPSR] = _cpsr;
  if(store_mask & 0xFF)
  {
    set_cpu_mode(cpu_modes[_cpsr & 0x1F]);
    if((io_registers[REG_IE] & io_registers[REG_IF]) &&
     io_registers[REG_IME] && ((_cpsr & 0x80) == 0))
    {
      reg_mode[MODE_IRQ][6] = address + 4;
      spsr[MODE_IRQ] = _cpsr;
      reg[REG_CPSR] = 0xD2;
      set_cpu_mode(MODE_IRQ);
      return 0x00000018;
    }
  }
  
  return 0;
}

#define arm_psr_read(op_type, psr_reg)                                        \
  generate_function_call(execute_read_##psr_reg);                             \
  generate_store_reg(rv, rd)                                                  \

  void function_cc execute_store_cpsr(u32 new_cpsr, u32 store_mask) {
    SH4_LOG(DBG_INFO, "Storing CPSR: %08x (mask %08x)\n", new_cpsr, store_mask);
    u32 old_mode = reg[CPU_MODE];
    reg[REG_CPSR] = (new_cpsr & store_mask) | (reg[REG_CPSR] & (~store_mask));
    
    // Handle mode switch if needed
    if((store_mask & 0xFF) && ((reg[REG_CPSR] & 0x1F) != old_mode)) {
        set_cpu_mode(reg[REG_CPSR] & 0x1F);
    }
    extract_flags();
}

void function_cc execute_store_spsr(u32 new_spsr, u32 store_mask)
{
  u32 _spsr = spsr[reg[CPU_MODE]];
  spsr[reg[CPU_MODE]] = (new_spsr & store_mask) | (_spsr & (~store_mask));
}

#define arm_psr_load_new_reg()                                                \
  generate_load_reg(a4, rm)                                                   \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(a4, imm)                                                  \

  #define arm_psr_store(op_type, psr_reg) \
  do { \
      SH4_LOG(DBG_INFO, "arm_psr_store: op_type=%s, psr_reg=%s, opcode=%08x, condition=%02x\n", \
              #op_type, #psr_reg, opcode, condition); \
      arm_psr_load_new_##op_type(); \
      generate_load_imm(a5, psr_masks[psr_field]); \
      generate_load_pc(a6, (pc + 4)); /* Pass PC+4 to execute_store_cpsr_body */ \
      generate_function_call(execute_store_##psr_reg); \
  } while (0)


  #define arm_psr(op_type, transfer_type, psr_reg) \
  do { \
      SH4_LOG(DBG_INFO, "arm_psr: op_type=%s, transfer_type=%s, psr_reg=%s, opcode=%08x, condition=%02x\n", \
              #op_type, #transfer_type, #psr_reg, opcode, condition); \
      u32 fixed_condition = condition & 0x0F; \
      if (condition >= 0x20) { \
          SH4_LOG(DBG_WARNING, "Invalid condition code %02x for opcode=%08x, pc=%08x, assuming NE\n", condition, opcode, pc); \
          fixed_condition = 0x1; /* Assume NE for PSR instructions */ \
      } \
      arm_decode_psr_##op_type(); \
      if (fixed_condition != 0x0E) { \
          generate_function_call(execute_read_cpsr); /* Returns CPSR in R0 */ \
          sh4_emit_mov_reg_reg(a4, rv); /* MOV R0, R4 */ \
          sh4_emit_test_reg_imm(a4, 1 << 30); /* Test Z flag */ \
          u16* skip_psr = (u16*)translation_ptr; \
          sh4_emit_word(fixed_condition == 0x1 ? sh4_opcode_bt : sh4_opcode_bf | 0x00); \
          sh4_emit_word(sh4_opcode_nop); \
          arm_psr_##transfer_type(op_type, psr_reg); \
          u16* end_psr = (u16*)translation_ptr; \
          s32 disp = (end_psr - skip_psr - 2) >> 1; \
          if (disp > 127 || disp < -128) { \
              SH4_LOG(DBG_ERROR, "Branch offset %d too large at %p\n", disp, skip_psr); \
              reg[CPU_HALT_STATE] = 1; \
          } else { \
              *skip_psr = (fixed_condition == 0x1 ? sh4_opcode_bt : sh4_opcode_bf) | (disp & 0xFF); \
          } \
      } else { \
          arm_psr_##transfer_type(op_type, psr_reg); \
      } \
  } while (0)
  
  #define arm_psr_store(op_type, psr_reg) \
  do { \
      SH4_LOG(DBG_INFO, "arm_psr_store: op_type=%s, psr_reg=%s, opcode=%08x, condition=%02x\n", \
              #op_type, #psr_reg, opcode, condition); \
      arm_psr_load_new_##op_type(); /* Loads new PSR value into R4 */ \
      generate_load_imm(a5, psr_masks[psr_field]); /* Mask into R5 */ \
      generate_load_pc(a6, (pc + 4)); /* PC+4 into R6 */ \
      generate_function_call(execute_store_##psr_reg); /* execute_store_cpsr(R4, R5, R6) */ \
  } while (0)

#define aligned_address_mask8  0xF0000000
#define aligned_address_mask16 0xF0000001
#define aligned_address_mask32 0xF0000003

#define read_memory(size, type, address, dest)                                \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask##size) == 0) &&                         \
   (map = memory_map_read[address >> 15]))                                    \
  {                                                                           \
    dest = *((type *)((u8 *)map + (address & 0x7FFF)));                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dest = (type)read_memory##size(address);                                  \
  }                                                                           \
}                                                                             \

#define read_memory_s16(address, dest)                                        \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask16) == 0) &&                             \
   (map = memory_map_read[address >> 15]))                                    \
  {                                                                           \
    dest = *((s16 *)((u8 *)map + (address & 0x7FFF)));                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dest = (s16)read_memory16_signed(address);                                \
  }                                                                           \
}                                                                             \

#define access_memory_generate_read_function(mem_size, mem_type)              \
u32 function_cc execute_load_##mem_type(u32 address)                          \
{                                                                             \
  SH4_LOG(DBG_INFO, "execute_load_%s: address=%08x\n", #mem_type, address);   \
  u32 dest;                                                                   \
  read_memory(mem_size, mem_type, address, dest);                             \
  return dest;                                                                \
}                                                                             \

access_memory_generate_read_function(8, u8);
access_memory_generate_read_function(8, s8);
access_memory_generate_read_function(16, u16);
access_memory_generate_read_function(32, u32);

u32 function_cc execute_load_s16(u32 address)
{
  u32 dest;
  read_memory_s16(address, dest);
  return dest;
}

#define access_memory_generate_write_function(mem_size, mem_type)             \
void function_cc execute_store_##mem_type(u32 address, u32 source)            \
{                                                                             \
  u8 *map;                                                                    \
  if(((address & aligned_address_mask##mem_size) == 0) &&                     \
   (map = memory_map_write[address >> 15]))                                   \
  {                                                                           \
    *((mem_type *)((u8 *)map + (address & 0x7FFF))) = source;                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    write_memory##mem_size(address, source);                                  \
  }                                                                           \
}                                                                             \

#define arm_access_memory_load(mem_type) \
do { \
    SH4_LOG(DBG_INFO, "arm_access_memory_load: mem_type=%s, opcode=%08x\n", #mem_type, opcode); \
    cycle_count += 2; \
    if (rn == REG_PC) { \
        u32 offset = opcode & 0xFFF; \
        u32 address = pc + 8 + (opcode & 0x00800000 ? offset : -offset); \
        sh4_emit_mov_reg_imm(a4, address); \
        SH4_LOG(DBG_INFO, "arm_access_memory_load: mem_type=%s, R%d from PC+%d = %08x\n", \
                #mem_type, rd, opcode & 0x00800000 ? offset : -offset, address); \
    } else { \
        generate_load_reg(a4, rn); \
        u32 offset = opcode & 0xFFF; \
        if (offset != 0) { \
            if (opcode & 0x00800000) { \
                sh4_emit_add_reg_imm(a4, offset); \
            } else { \
                sh4_emit_sub_reg_imm(a4, offset); \
            } \
        } \
    } \
    generate_function_call(execute_load_##mem_type); /* execute_load_##mem_type(R4) -> R0 */ \
    sh4_emit_mov_reg_reg(a4, rv); /* MOV R0, R4 to preserve result */ \
    generate_store_reg_pc_no_flags(a4, rd);\
} while (0)



  #define arm_access_memory_store(mem_type)                                     \
SH4_LOG(DBG_INFO, "arm_access_memory_store: mem_type=%s, opcode=%08x\n", #mem_type, opcode); \
  cycle_count += 2;                                                              \
  generate_load_reg_pc(a5, rd, 12);                                           \
  generate_load_pc(a4, (pc + 4));                                             \
  generate_function_call(execute_store_##mem_type)                            \

#define no_op                                                                 \

#define arm_access_memory_writeback_yes(off_op)                               \
  reg[rn] = address off_op                                                    \

#define arm_access_memory_writeback_no(off_op)                                \

#define load_reg_op reg[rd]                                                   \

#define store_reg_op reg_op                                                   \

#define arm_access_memory_adjust_op_up      add
#define arm_access_memory_adjust_op_down    sub
#define arm_access_memory_reverse_op_up     sub
#define arm_access_memory_reverse_op_down   add

#define arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a4, rn, 8);                                            \
  generate_##adjust_dir_op(a4, a5)                                            \

#define arm_access_memory_reg_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a4, rn)                                                  \

#define arm_access_memory_reg_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a4, rn);                                                  \
  generate_##adjust_dir_op(a4, a5);                                           \
  generate_store_reg(a4, rn);                                                 \
  generate_##reverse_dir_op(a4, a5)                                           \

#define arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a4, rn, 8);                                            \
  generate_##adjust_dir_op##_imm(a4, offset)                                  \

#define arm_access_memory_imm_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a4, rn)                                                  \

#define arm_access_memory_imm_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a4, rn);                                                  \
  generate_##adjust_dir_op##_imm(a4, offset);                                 \
  generate_store_reg(a4, rn);                                                 \
  generate_##reverse_dir_op##_imm(a4, offset)                                 \


#define arm_data_trans_reg(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_reg();                                                \
  generate_load_offset_sh();                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_imm(adjust_op, adjust_dir_op, reverse_dir_op)          \
  arm_decode_data_trans_imm();                                                \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_reg(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_r();                                                  \
  generate_load_reg(a1, rm);                                                  \
  arm_access_memory_reg_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_data_trans_half_imm(adjust_op, adjust_dir_op, reverse_dir_op)     \
  arm_decode_half_trans_of();                                                 \
  arm_access_memory_imm_##adjust_op(adjust_dir_op, reverse_dir_op)            \

#define arm_access_memory(access_type, direction, adjust_op, mem_type,        \
    offset_type)                                                                 \
   {                                                                             \
     arm_data_trans_##offset_type(adjust_op,                                     \
      arm_access_memory_adjust_op_##direction,                                   \
      arm_access_memory_reverse_op_##direction);                                 \
                                                                                 \
     arm_access_memory_##access_type(mem_type);                                  \
   }                                                                             \

#define word_bit_count(word)                                                  \
   (bit_count[word >> 8] + bit_count[word & 0xFF])                             \

#define arm_block_address_preadjust_up_full()                                 \
  generate_add_imm(s0, (word_bit_count(reg_list)));                        \

#define arm_block_address_preadjust_up()                                      \
  generate_add_imm(s0, 4);                                                     \

#define arm_block_address_preadjust_down_full()                               \
  generate_sub_imm(s0, (word_bit_count(reg_list)))                        \

#define arm_block_address_preadjust_down()                                    \
  generate_sub_imm(s0, ((word_bit_count(reg_list)) - 4))                  \

#define arm_block_address_preadjust_no()                                      \

#define arm_block_address_postadjust_no()                                     \

#define arm_block_address_postadjust_up()                                     \
  generate_add_imm(a0, (word_bit_count(reg_list)))                        \

#define arm_block_address_postadjust_down()                                   \
  generate_sub_imm(a0, (word_bit_count(reg_list)))                        \

#define sprint_no(access_type, pre_op, post_op, wb)                           \

#define sprint_yes(access_type, pre_op, post_op, wb)                          \
  /* printf("sbit on %s %s %s %s\n", #access_type, #pre_op, #post_op, #wb)*/   \

#define arm_block_writeback_yes(access_type)                                  \
  generate_store_reg(a0, rn)                                                  \

#define arm_block_writeback_no(access_type)                                   \

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

#define arm_block_memory_load()                                               \
SH4_LOG(DBG_INFO, "arm_block_memory_load: i=%d, pc=%08x\n", i, pc);               \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, i)                                                   \

#define arm_block_memory_store()                                              \
SH4_LOG(DBG_INFO, "arm_block_memory_store: i=%d, pc=%08x\n", i, pc);               \
  generate_load_reg_pc(a4, i, 8);                                             \
  generate_load_reg(a5, i);                                                  \
  generate_function_call(execute_aligned_store32)

#define arm_block_memory_final_load()                                         \
SH4_LOG(DBG_INFO, "arm_block_memory_final_load: i=%d, pc=%08x\n", i, pc); \
  arm_block_memory_load()                                                     \

  #define arm_block_memory_final_store()                                        \
  SH4_LOG(DBG_INFO, "arm_block_memory_final_store: i=%d, pc=%08x\n", i, pc); \
  generate_load_reg_pc(a4, i, 12);                                            \
  generate_load_pc(a5, (pc + 4));                                             \
  generate_function_call(execute_store_u32)


#define arm_block_memory_adjust_pc_store()                                    \

#define arm_block_memory_adjust_pc_load()                                     \
  SH4_LOG(DBG_INFO, "arm_block_memory_adjust_pc_load: i=%d, pc=%08x\n", i, pc); \
  if(reg_list & 0x8000)                                                       \
  {                                                                           \
    generate_mov(a4, rv);                                                     \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_offset_down_a()                                      \
{\
  generate_add_imm(s0, -((word_bit_count(reg_list)) - 4));                 \
}

#define arm_block_memory_offset_down_b()                                      \
{\
  generate_add_imm(s0, -(word_bit_count(reg_list)));                       \
}
#define arm_block_memory_offset_no()                                          \

#define arm_block_memory_offset_up()                                          \
{\
  generate_add_imm(s0, 4);                                                     \
}

#define arm_block_memory_writeback_down()                                     \
{\
  generate_load_reg(a0, rn);                                                   \
  generate_add_imm(a0, -(word_bit_count(reg_list)));                      \
  generate_store_reg(a0, rn);                                                  \
}

#define arm_block_memory_writeback_up()                                       \
{\
  generate_load_reg(a0, rn);                                                  \
  generate_add_imm(a0, (word_bit_count(reg_list)));                       \
  generate_store_reg(a0, rn);                                                  \
}

#define arm_block_memory_writeback_no()                                       \

#define arm_block_memory_writeback_load(writeback_type)                       \
{\
  if(!((reg_list >> rn) & 0x01))                                              \
  {                                                                           \
    arm_block_memory_writeback_##writeback_type();                            \
  }                                                                           \
}

#define arm_block_memory_offset_no()                                          \

#define arm_block_memory_offset_up()                                          \
{\
  generate_add_imm(s0, 4);                                                     \
}

#define arm_block_memory_writeback_down()                                     \
{\
  generate_load_reg(a0, rn);                                                   \
  generate_add_imm(a0, -(word_bit_count(reg_list)));                      \
  generate_store_reg(a0, rn);                                                  \
}

#define arm_block_memory_writeback_up()                                       \
{\
  generate_load_reg(a0, rn);                                                  \
  generate_add_imm(a0, (word_bit_count(reg_list)));                       \
  generate_store_reg(a0, rn);                                                  \
}

#define arm_block_memory_writeback_no()



#define arm_block_memory_writeback_load(writeback_type)                       \
{\
  if(!((reg_list >> rn) & 0x01))                                              \
  {                                                                           \
    arm_block_memory_writeback_##writeback_type();                            \
  }                                                                           \
}

#define arm_block_memory_writeback_store(writeback_type)                      \
{\
  arm_block_memory_writeback_##writeback_type();                               \
}

#define arm_block_memory(access_type, offset_type, writeback_type, s_bit)     \
{                                                                             \
    arm_decode_block_trans();                                                 \
    u32 offset = 0;                                                           \
    u32 i;                                                                    \
                                                                              \
    generate_load_reg(s0, rn); /* base register into s0 */                   \
    arm_block_memory_offset_##offset_type();                                  \
    arm_block_memory_writeback_##access_type(writeback_type);                 \
    generate_and_imm(s0, ~0x03);                                              \
                                                                              \
    for(i = 0; i < 16; i++)                                                   \
    {                                                                         \
      if((reg_list >> i) & 0x01)                                              \
      {                                                                       \
        cycle_count++;                                                        \
        generate_add_reg_reg_imm(a4, s0, offset); /* a4 = s0 + offset */     \
        if(reg_list & ~((2 << i) - 1))                                        \
        {                                                                     \
          arm_block_memory_##access_type();                                   \
          offset += 4;                                                        \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          arm_block_memory_final_##access_type();                             \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    arm_block_memory_adjust_pc_##access_type();                               \
}                                                                          \

#define arm_swap(type)                                                        \
{    \
  SH4_LOG(DBG_INFO, "arm_swap: type=%s, opcode=%08x, pc=%08x\n", #type, opcode, pc); \
  arm_decode_swap();                                                          \
  cycle_count += 3;                                                           \
  generate_load_reg(a4, rn);                                                  \
  generate_function_call(execute_load_##type);                                \
  generate_mov(s0, rv);                                                       \
  generate_load_reg(a4, rn);                                                  \
  generate_load_reg(a5, rm);                                                  \
  generate_function_call(execute_store_##type);                               \
  generate_store_reg(s0, rd);                                                 \
}


#define thumb_rn_op_reg(_rn)                                                  \
  generate_load_reg(a0, _rn)                                                  \

#define thumb_rn_op_imm(_imm)                                                 \
  generate_load_imm(a0, _imm)                                                 \

// Types: add_sub, add_sub_imm, alu_op, imm
// Affects N/Z/C/V flags

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  generate_function_call(execute_##name);                                     \
  generate_store_reg(rv, _rd);                                                \
}                                                                             \

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_load_reg(a1, _rs);                                                 \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_function_call(execute_##name);                                     \
  generate_store_reg(rv, _rd);                                                \
}                                                                             \

#define thumb_data_proc_mov(type, rn_type, _rd, _rn)                          \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_rn_op_##rn_type(_rn);                                                 \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define generate_store_reg_pc_thumb(ireg)                                     \
  generate_store_reg(ireg, rd);                                               \
  if(rd == 15)                                                                \
  {                                                                           \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_thumb(rv);                                            \
}                                                                             \

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_load_reg_pc(a1, rd, 4);                                            \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define thumb_data_proc_unary_hi(name)                                        \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rn, 4);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_thumb(rv);                                            \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_store_reg_pc_thumb(a0);                                            \
}                                                                             \

#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_pc(a0, (((pc & ~2) + 4) + (imm)));                        \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (imm));                                            \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_adjust_sp(value)                                                \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (value));                                              \
  generate_store_reg(a0, 13);                                                 \
}                                                                             \

#define generate_shift_load_operands_reg()                                    \
  generate_load_reg(a0, rd);                                                  \
  generate_load_reg(a1, rs)                                                   \

#define generate_shift_load_operands_imm()                                    \
  generate_load_reg(a0, rs);                                                  \
  generate_load_imm(a1, imm)                                                  \

#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
  generate_shift_load_operands_##value_type();                                \
  generate_function_call(execute_##op_type##_##value_type##_op);              \
  generate_store_reg(rv, rd);                                                 \
}                                                                             \

// Operation types: imm, mem_reg, mem_imm

#define thumb_access_memory_load(mem_type, reg_rd)                            \
  cycle_count += 2; \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg(rv, reg_rd)                                              \

  #define thumb_access_memory_store(mem_type, reg_rd)                           \
  cycle_count++;                                                              \
  generate_load_reg(a5, reg_rd);                                              \
  generate_load_pc(a4, (pc + 2));                                             \
  generate_function_call(execute_store_##mem_type)


#define thumb_access_memory_generate_address_pc_relative(offset, reg_rb,      \
 reg_ro)                                                                      \
  generate_load_pc(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_imm(offset, reg_rb, reg_ro)  \
  generate_load_reg(a0, reg_rb);                                              \
  generate_add_imm(a0, (offset))                                              \

#define thumb_access_memory_generate_address_reg_reg(offset, reg_rb, reg_ro)  \
  generate_load_reg(a0, reg_rb);                                              \
  generate_load_reg(a1, reg_ro);                                              \
  generate_add(a0, a1)                                                        \

#define thumb_access_memory(access_type, op_type, reg_rd, reg_rb, reg_ro,     \
 address_type, offset, mem_type)                                              \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, reg_rb,         \
   reg_ro);                                                                   \
  thumb_access_memory_##access_type(mem_type, reg_rd);                        \
}                                                                             \

#define thumb_block_address_preadjust_up()                                    \
  generate_add_imm(s0, (bit_count[reg_list]))                             \

#define thumb_block_address_preadjust_down()                                  \
  generate_sub_imm(s0, (bit_count[reg_list]))                             \

#define thumb_block_address_preadjust_push_lr()                               \
  generate_sub_imm(s0, ((bit_count[reg_list] + 1)))                       \

#define thumb_block_address_preadjust_no()                                    \

#define thumb_block_address_postadjust_no(base_reg)                           \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_address_postadjust_up(base_reg)                           \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list]));                \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_down(base_reg)                         \
  generate_mov(a0, s0);                                                       \
  generate_sub_imm(a0, (bit_count[reg_list]));                            \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  generate_add_reg_reg_imm(a0, s0, ((bit_count[reg_list] + 1)));          \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_pop_pc()                                     \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list]));                \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, REG_PC);                                             \
  generate_mov(a0, rv);                                                       \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_extra_push_lr(base_reg)                            \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list]));                \
  generate_load_reg(a1, REG_LR);                                              \
  generate_function_call(execute_aligned_store32)                             \

#define thumb_block_memory_load()                                             \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, i)                                                   \

#define thumb_block_memory_store()                                            \
  generate_load_reg(a1, i);                                                   \
  generate_function_call(execute_aligned_store32)                             \

#define thumb_block_memory_final_load()                                       \
  thumb_block_memory_load()                                                   \

  #define thumb_block_memory_final_store()                                      \
  generate_load_reg(a5, i);                                                   \
  generate_load_pc(a4, (pc + 2));                                             \
  generate_function_call(execute_store_u32)


#define thumb_block_memory_final_no(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_up(access_type)                              \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_down(access_type)                            \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_push_lr(access_type)                         \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory_final_pop_pc(access_type)                          \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory(access_type, pre_op, post_op, base_reg)            \
{                                                                             \
  thumb_decode_rlist();                                                       \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
                                                                              \
  generate_load_reg(s0, base_reg);                                            \
  generate_and_imm(s0, ~0x03);                                                \
  thumb_block_address_preadjust_##pre_op();                                   \
  thumb_block_address_postadjust_##post_op(base_reg);                         \
                                                                              \
  for(i = 0; i < 8; i++)                                                      \
  {                                                                           \
    if((reg_list >> i) & 0x01)                                                \
    {     \
      cycle_count++; \                                                                    
      generate_add_reg_reg_imm(a0, s0, offset)                                \
      if(reg_list & ~((2 << i) - 1))                                          \
      {                                                                       \
        thumb_block_memory_##access_type();                                   \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        thumb_block_memory_final_##post_op(access_type);                      \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  thumb_block_memory_extra_##post_op();                                       \
}                                                                             \


#define thumb_conditional_branch(condition) \
{ \
  condition_check_type condition_check; \
  generate_cycle_update(); \
  generate_condition_##condition(a4, a5); \
  generate_conditional_branch_type(a4, a5); \
  generate_branch_no_cycle_update( \
   block_exits[block_exit_position].branch_source, \
   block_exits[block_exit_position].branch_target); \
  generate_branch_patch_conditional(backpatch_address, translation_ptr); \
  block_exit_position++; \
}\

#define flags_vars(a, b) \
  u32 dest; \
  { const u32 _sa = (a); const u32 _sb = (b);

#define data_proc_generate_logic_function(name, expr)                         \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  return expr;                                                                \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  return expr;                                                                \
}                                                                             \

#define data_proc_generate_logic_unary_function(name, expr)                   \
u32 function_cc execute_##name(u32 rm)                                        \
{                                                                             \
  return expr;                                                                \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm)                                     \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
  return expr;                                                                \
}                                                                             \

#define data_proc_generate_sub_function(name, src_a_expr, src_b_expr)         \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  return _a - _b;                                                             \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  u32 dest;                                                                   \
  { const u32 _sa = _a; const u32 _sb = _b;                                    \
    dest = _a - _b;                                                           \
    calculate_flags_sub(dest, _sa, _sb);                                      \
  }                                                                           \
  return dest;                                                                \
}

#define data_proc_generate_add_function(name, src_a_expr, src_b_expr)         \
u32 function_cc execute_##name(u32 rm, u32 rn)                                \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  return _a + _b;                                                             \
}                                                                             \
                                                                              \
u32 function_cc execute_##name##s(u32 rm, u32 rn)                             \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  u32 dest;                                                                   \
  { const u32 _sa = _a; const u32 _sb = _b;                                    \
    dest = _sa + _sb;                                                         \
    calculate_flags_add(dest, _sa, _sb);                                      \
  }                                                                           \
  return dest;                                                                \
}

#define data_proc_generate_sub_test_function(name, src_a_expr, src_b_expr)    \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  { const u32 _sa = _a; const u32 _sb = _b;                                    \
    u32 dest = _sa - _sb;                                                     \
    calculate_flags_sub(dest, _sa, _sb);                                      \
  }                                                                           \
}

#define data_proc_generate_add_test_function(name, src_a_expr, src_b_expr)    \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  u32 _a = (src_a_expr);                                                      \
  u32 _b = (src_b_expr);                                                      \
  { const u32 _sa = _a; const u32 _sb = _b;                                    \
    u32 dest = _sa + _sb;                                                     \
    calculate_flags_add(dest, _sa, _sb);                                      \
  }                                                                           \
}

#define data_proc_generate_logic_test_function(name, expr)                    \
void function_cc execute_##name(u32 rm, u32 rn)                               \
{                                                                             \
  u32 dest = expr;                                                            \
  calculate_z_flag(dest);                                                     \
  calculate_n_flag(dest);                                                     \
}                                                                             \

u32 function_cc execute_neg(u32 rm)                                           \
{                                                                             \
  u32 dest = 0 - rm;                                                          \
  calculate_flags_sub(dest, 0, rm);                                           \
  return dest;                                                                \
}                                                                             \

// Execute functions

data_proc_generate_logic_function(and, rn & rm);
data_proc_generate_logic_function(eor, rn ^ rm);
data_proc_generate_logic_function(orr, rn | rm);
data_proc_generate_logic_function(bic, rn & (~rm));
data_proc_generate_logic_function(mul, rn * rm);
data_proc_generate_logic_unary_function(mov, rm);
data_proc_generate_logic_unary_function(mvn, ~rm);

data_proc_generate_sub_function(sub, rn, rm);
data_proc_generate_sub_function(rsb, rm, rn);
data_proc_generate_sub_function(sbc, rn, (rm + (reg[REG_C_FLAG] ^ 1)));
data_proc_generate_sub_function(rsc, (rm + reg[REG_C_FLAG] - 1), rn);
data_proc_generate_add_function(add, rn, rm);
data_proc_generate_add_function(adc, rn, rm + reg[REG_C_FLAG]);

data_proc_generate_logic_test_function(tst, rn & rm);
data_proc_generate_logic_test_function(teq, rn ^ rm);
data_proc_generate_sub_test_function(cmp, rn, rm);
data_proc_generate_add_test_function(cmn, rn, rm);

static void function_cc execute_swi(u32 pc)
{
  reg_mode[MODE_SUPERVISOR][6] = pc;
  collapse_flags();
  spsr[MODE_SUPERVISOR] = reg[REG_CPSR];
  reg[REG_CPSR] = (reg[REG_CPSR] & ~0x3F) | 0x13;
  set_cpu_mode(MODE_SUPERVISOR);
}

#define arm_conditional_block_header() \
do { \
  SH4_LOG(DBG_INFO, "arm_conditional_block_header: PC=%08x\n", pc); \
  backpatch_address = translation_ptr; \
  generate_condition(a0, a1);                                                 \
  generate_conditional_branch_type(a0, a1);                                   \
} while(0)


u32 arm_decode_branch_target(u32 pc, u32 opcode) {
  s32 offset = (s32)(opcode & 0x00FFFFFF);
  
  /* Sign extend 24-bit offset */
  offset = (offset << 8) >> 8;  // More efficient sign extension
  
  offset <<= 2;  // Convert to byte offset
  
  u32 target = pc + 8 + offset;
  
  /* Validate target address */
  if (target >= 0x10000000) {  // Example range check
      SH4_LOG(DBG_INFO, "Invalid branch target %08x\n", target);
      return 0;  // Or handle error differently
  }
  
  return target;
}

#define arm_b() \
do { \
    u32 branch_target = arm_decode_branch_target(pc, opcode); \
    SH4_LOG(DBG_INFO, "arm_b: PC=%08x -> %08x, target=%08x, source=%08x\n", pc, branch_target, block_exits[block_exit_position].branch_target, block_exits[block_exit_position].branch_source); \
    generate_branch(); \
} while (0)

#define arm_bl()                                                              \
  SH4_LOG(DBG_INFO, "arm_bl: PC=%08x\n", pc);                               \
  generate_update_pc((pc + 4));                                               \
  generate_store_reg(a4, REG_LR);                                             \
  generate_branch()                                                           \
  
// 🏹 Branch Exchange (BX - jump indirect)
#define arm_bx() \
  do { \
      SH4_LOG(DBG_INFO, "arm_bx: PC=%08x\n", pc); \
      arm_decode_branchx(); \
      SH4_LOG(DBG_INFO, "arm_bx: Jump via R%d at PC=%08x\n", rn, pc); \
      generate_load_reg(a4,rn); \
      generate_indirect_branch_dual(); \
  } while (0)


#define arm_swi()                                                             \
  generate_swi_hle_handler((opcode >> 16) & 0xFF);                            \
  generate_update_pc((pc + 4));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch()                                                           \

#define thumb_b()                                                             \
{\
generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++;                                                       \
}

#define thumb_bl()                                                            \
{\
  generate_update_pc(((pc + 2) | 0x01));                                      \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++;                                                       \
}

#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  generate_load_reg(a1, REG_LR);                                              \
  generate_store_reg(a0, REG_LR);                                             \
  generate_mov(a0, a1);                                                       \
  generate_add_imm(a0, (offset * 2));                                         \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  generate_load_reg_pc(a0, rs, 4);                                            \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define thumb_swi()                                                           \
  generate_swi_hle_handler(opcode & 0xFF);                                    \
  generate_update_pc((pc + 2));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++;                                                       \

u8 swi_hle_handle[256] =
{
  0x0,    // SWI 0:  SoftReset
  0x0,    // SWI 1:  RegisterRAMReset
  0x0,    // SWI 2:  Halt
  0x0,    // SWI 3:  Stop/Sleep
  0x0,    // SWI 4:  IntrWait
  0x0,    // SWI 5:  VBlankIntrWait
  0x1,    // SWI 6:  Div
  0x0,    // SWI 7:  DivArm
  0x0,    // SWI 8:  Sqrt
  0x0,    // SWI 9:  ArcTan
  0x0,    // SWI A:  ArcTan2
  0x0,    // SWI B:  CpuSet
  0x0,    // SWI C:  CpuFastSet
  0x0,    // SWI D:  GetBIOSCheckSum
  0x0,    // SWI E:  BgAffineSet
  0x0,    // SWI F:  ObjAffineSet
  0x0,    // SWI 10: BitUnpack
  0x0,    // SWI 11: LZ77UnCompWram
  0x0,    // SWI 12: LZ77UnCompVram
  0x0,    // SWI 13: HuffUnComp
  0x0,    // SWI 14: RLUnCompWram
  0x0,    // SWI 15: RLUnCompVram
  0x0,    // SWI 16: Diff8bitUnFilterWram
  0x0,    // SWI 17: Diff8bitUnFilterVram
  0x0,    // SWI 18: Diff16bitUnFilter
  0x0,    // SWI 19: SoundBias
  0x0,    // SWI 1A: SoundDriverInit
  0x0,    // SWI 1B: SoundDriverMode
  0x0,    // SWI 1C: SoundDriverMain
  0x0,    // SWI 1D: SoundDriverVSync
  0x0,    // SWI 1E: SoundChannelClear
  0x0,    // SWI 1F: MidiKey2Freq
  0x0,    // SWI 20: SoundWhatever0
  0x0,    // SWI 21: SoundWhatever1
  0x0,    // SWI 22: SoundWhatever2
  0x0,    // SWI 23: SoundWhatever3
  0x0,    // SWI 24: SoundWhatever4
  0x0,    // SWI 25: MultiBoot
  0x0,    // SWI 26: HardReset
  0x0,    // SWI 27: CustomHalt
  0x0,    // SWI 28: SoundDriverVSyncOff
  0x0,    // SWI 29: SoundDriverVSyncOn
  0x0     // SWI 2A: SoundGetJumpList
};

void swi_hle_div()
{
  s32 result = (s32)reg[0] / (s32)reg[1];
  reg[0] = result;
  reg[1] = (s32)reg[0] % (s32)reg[1];
  reg[3] = (result ^ (result >> 31)) - (result >> 31);
}

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

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


#define generate_swi_hle_handler(_swi_number)                                 \
{ \
{                                                                             \
  u32 swi_number = _swi_number;                                               \
  if(swi_hle_handle[swi_number])                                              \
  {                                                                           \
    /* Div */                                                                 \
    if(swi_number == 0x06)                                                    \
    {                                                                         \
      generate_function_call(swi_hle_div);                                    \
    }                                                                         \
    break;                                                                    \
  }                                                                           \
}                                                                             \
}

#define generate_translation_gate(type)                                       \
{\
  generate_update_pc(pc);                                                     \
  generate_indirect_branch_no_cycle_update(type);                              \
}

#define generate_step_debug()                                                 \
{\
  generate_load_imm(a4, pc);                                                  \
  generate_function_call(step_debug_sh4);                                      \
}

u32 function_cc execute_mul_long_u64(u32 rm, u32 rs);
u32 function_cc execute_mul_long_s64(u32 rm, u32 rs);

#endif