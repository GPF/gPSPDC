#ifndef SH4_EMIT_H
#define SH4_EMIT_H

#include "common.h"
#include "cpu.h"

typedef u16 *translation_ptr_t;
extern translation_ptr_t translation_ptr;
void sh4_invalidate_icache_region(u32 addr, u32 size);

typedef enum {
  sh4_reg_r0 = 0, sh4_reg_r1 = 1, sh4_reg_r2 = 2, sh4_reg_r3 = 3,
  sh4_reg_r4 = 4, sh4_reg_r5 = 5, sh4_reg_r6 = 6, sh4_reg_r7 = 7,
  sh4_reg_r8 = 8, sh4_reg_r9 = 9, sh4_reg_r10 = 10, sh4_reg_r11 = 11,
  sh4_reg_r12 = 12, sh4_reg_r13 = 13, sh4_reg_r14 = 14, sh4_reg_r15 = 15
} sh4_reg_number;

#define REG_BASE    sh4_reg_r12
#define REG_CYCLES  sh4_reg_r13
#define REG_TEMP    sh4_reg_r1
#define REG_A0      sh4_reg_r4
#define REG_A1      sh4_reg_r5
#define REG_A2      sh4_reg_r6
#define REG_A3      sh4_reg_r7
#define REG_RV      sh4_reg_r0
#define REG_S0      sh4_reg_r14

#define sh4_reg_a0 sh4_reg_r4
#define sh4_reg_a1 sh4_reg_r5
#define sh4_reg_a2 sh4_reg_r6
#define sh4_reg_rv sh4_reg_r0
#define sh4_reg_s0 sh4_reg_r14

#define SH4_IREG(ireg) sh4_reg_##ireg

#define SH4_EMIT_BYTE(value) \
  *(translation_ptr++) = (value)

#define SH4_EMIT_NOP() \
  SH4_EMIT_BYTE(0x0009)

#define SH4_RELATIVE_OFFSET(source, target) \
  (((u32)(target) - ((u32)(source) + 4)) >> 1)

#define SH4_EMIT_MOV(rd, rm) \
  SH4_EMIT_BYTE(0x6003 | ((rd & 0xF) << 8) | ((rm & 0xF) << 4))

#define SH4_EMIT_MOVI(rd, imm) \
  SH4_EMIT_BYTE(0xE000 | ((rd & 0xF) << 8) | ((imm) & 0xFF))

#define SH4_EMIT_ADD(rd, rn, rm) \
  do { \
    if((rd) == (rn)) { \
      SH4_EMIT_BYTE(0x300C | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } else if((rd) == (rm)) { \
      SH4_EMIT_BYTE(0x300C | (((rd) & 0xF) << 8) | (((rn) & 0xF) << 4)); \
    } else { \
      SH4_EMIT_MOV((rd), (rn)); \
      SH4_EMIT_BYTE(0x300C | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } \
  } while(0)

#define SH4_EMIT_SUB(rd, rn, rm) \
  do { \
    if((rd) == (rn)) { \
      SH4_EMIT_BYTE(0x3008 | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } else if((rd) == (rm)) { \
      SH4_EMIT_NOT((rd), (rd)); \
      SH4_EMIT_ADD((rd), (rn), (rd)); \
      SH4_EMIT_ADDI8((rd), 1); \
    } else { \
      SH4_EMIT_MOV((rd), (rn)); \
      SH4_EMIT_BYTE(0x3008 | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } \
  } while(0)

#define SH4_EMIT_OR(rd, rn, rm) \
  do { \
    if((rd) == (rn)) { \
      SH4_EMIT_BYTE(0x200B | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } else if((rd) == (rm)) { \
      SH4_EMIT_BYTE(0x200B | (((rd) & 0xF) << 8) | (((rn) & 0xF) << 4)); \
    } else { \
      SH4_EMIT_MOV((rd), (rn)); \
      SH4_EMIT_BYTE(0x200B | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } \
  } while(0)

#define SH4_EMIT_AND(rd, rn, rm) \
  do { \
    if((rd) == (rn)) { \
      SH4_EMIT_BYTE(0x2009 | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } else if((rd) == (rm)) { \
      SH4_EMIT_BYTE(0x2009 | (((rd) & 0xF) << 8) | (((rn) & 0xF) << 4)); \
    } else { \
      SH4_EMIT_MOV((rd), (rn)); \
      SH4_EMIT_BYTE(0x2009 | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } \
  } while(0)

#define SH4_EMIT_XOR(rd, rn, rm) \
  do { \
    if((rd) == (rn)) { \
      SH4_EMIT_BYTE(0x200A | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } else if((rd) == (rm)) { \
      SH4_EMIT_BYTE(0x200A | (((rd) & 0xF) << 8) | (((rn) & 0xF) << 4)); \
    } else { \
      SH4_EMIT_MOV((rd), (rn)); \
      SH4_EMIT_BYTE(0x200A | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4)); \
    } \
  } while(0)

#define SH4_EMIT_NOT(rd, rm) \
  SH4_EMIT_BYTE(0x6007 | ((rd & 0xF) << 8) | ((rm & 0xF) << 4))

#define SH4_EMIT_LW(rd, rn, offset) \
  SH4_EMIT_BYTE(0x5000 | (((rd) & 0xF) << 8) | (((rn) & 0xF) << 4) | (((offset) >> 2) & 0xF))

#define SH4_EMIT_SW(rd, rn, offset) \
  SH4_EMIT_BYTE(0x1000 | (((rn) & 0xF) << 8) | (((rd) & 0xF) << 4) | (((offset) >> 2) & 0xF))

#define SH4_EMIT_LOAD_MEM_W(rd, rn, byte_offset) \
  do { \
    u32 _byte_off = (u32)(byte_offset); \
    if(_byte_off <= 60 && ((_byte_off & 3) == 0)) { \
      SH4_EMIT_LW(rd, rn, _byte_off); \
    } else { \
      SH4_EMIT_LOAD_IMM(sh4_reg_r2, _byte_off); \
      SH4_EMIT_ADD(sh4_reg_r2, rn, sh4_reg_r2); \
      SH4_EMIT_LW(rd, sh4_reg_r2, 0); \
    } \
  } while(0)

#define SH4_EMIT_STORE_MEM_W(rd, rn, byte_offset) \
  do { \
    u32 _byte_off = (u32)(byte_offset); \
    if(_byte_off <= 60 && ((_byte_off & 3) == 0)) { \
      SH4_EMIT_SW(rd, rn, _byte_off); \
    } else { \
      SH4_EMIT_LOAD_IMM(sh4_reg_r2, _byte_off); \
      SH4_EMIT_ADD(sh4_reg_r2, rn, sh4_reg_r2); \
      SH4_EMIT_SW(rd, sh4_reg_r2, 0); \
    } \
  } while(0)

#define SH4_EMIT_ADDI8(rn, imm) \
  SH4_EMIT_BYTE(0x7000 | (((rn) & 0xF) << 8) | ((imm) & 0xFF))

#define SH4_EMIT_ADDI(rd, rn, imm) \
  do { \
    s32 _addi = (s32)(imm); \
    if((rd) != (rn)) SH4_EMIT_MOV((rd), (rn)); \
    if(_addi >= -128 && _addi <= 127) { \
      SH4_EMIT_ADDI8((rd), _addi); \
    } else { \
      sh4_reg_number _tmp = ((rd) == sh4_reg_r1) ? sh4_reg_r2 : sh4_reg_r1; \
      SH4_EMIT_LOAD_IMM(_tmp, _addi); \
      SH4_EMIT_ADD((rd), (rd), _tmp); \
    } \
  } while(0)

#define SH4_EMIT_SHLL1(rd) \
  SH4_EMIT_BYTE(0x4000 | (((rd) & 0xF) << 8))

#define SH4_EMIT_SHLR1(rd) \
  SH4_EMIT_BYTE(0x4001 | (((rd) & 0xF) << 8))

#define SH4_EMIT_SHAR1(rd) \
  SH4_EMIT_BYTE(0x4021 | (((rd) & 0xF) << 8))

#define SH4_EMIT_ROTR1(rd) \
  SH4_EMIT_BYTE(0x4005 | (((rd) & 0xF) << 8))

#define SH4_EMIT_SHLL8(rd) \
  SH4_EMIT_BYTE(0x4018 | (((rd) & 0xF) << 8))

#define SH4_EMIT_EXTU_B(rd, rm) \
  SH4_EMIT_BYTE(0x600C | (((rd) & 0xF) << 8) | (((rm) & 0xF) << 4))

#define SH4_EMIT_JSR(rn) \
  do { \
    SH4_EMIT_BYTE(0x400B | (((rn) & 0xF) << 8)); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_JMP(rn) \
  do { \
    SH4_EMIT_BYTE(0x402B | ((rn & 0xF) << 8)); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_BRA_FILLER(writeback_location) \
  do { \
    (writeback_location) = translation_ptr; \
    SH4_EMIT_BYTE(0xA000); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_BT_FILLER(writeback_location) \
  do { \
    (writeback_location) = translation_ptr; \
    SH4_EMIT_BYTE(0x8900); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_BF_FILLER(writeback_location) \
  do { \
    (writeback_location) = translation_ptr; \
    SH4_EMIT_BYTE(0x8B00); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_LOAD_U8(rd, imm) \
  do { \
    u32 _u8 = (u32)(imm) & 0xFF; \
    SH4_EMIT_MOVI((rd), _u8); \
    if(_u8 & 0x80) SH4_EMIT_EXTU_B((rd), (rd)); \
  } while(0)

#define SH4_EMIT_ADD_UNSIGNED_BYTE(rd, imm) \
  do { \
    u32 _add_byte = (u32)(imm) & 0xFF; \
    while(_add_byte > 127) { \
      SH4_EMIT_ADDI8((rd), 127); \
      _add_byte -= 127; \
    } \
    if(_add_byte != 0) SH4_EMIT_ADDI8((rd), _add_byte); \
  } while(0)

#define SH4_EMIT_LOAD_IMM(rd, imm) \
  do { \
    u32 _imm = (u32)(imm); \
    s32 _signed_imm = (s32)_imm; \
    if(_signed_imm >= -128 && _signed_imm <= 127) { \
      SH4_EMIT_MOVI((rd), _imm); \
    } else { \
      SH4_EMIT_LOAD_U8((rd), _imm >> 24); \
      SH4_EMIT_SHLL8((rd)); \
      SH4_EMIT_ADD_UNSIGNED_BYTE((rd), _imm >> 16); \
      SH4_EMIT_SHLL8((rd)); \
      SH4_EMIT_ADD_UNSIGNED_BYTE((rd), _imm >> 8); \
      SH4_EMIT_SHLL8((rd)); \
      SH4_EMIT_ADD_UNSIGNED_BYTE((rd), _imm); \
    } \
  } while(0)

#define SH4_EMIT_LOAD_REG(ireg, reg_index) \
  SH4_EMIT_LOAD_MEM_W(ireg, REG_BASE, (reg_index) * 4)

#define SH4_EMIT_STORE_REG(ireg, reg_index) \
  SH4_EMIT_STORE_MEM_W(ireg, REG_BASE, (reg_index) * 4)

#define SH4_EMIT_FUNCTION_CALL(func) \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r1, (u32)(func)); \
    SH4_EMIT_JSR(sh4_reg_r1); \
  } while(0)

#define SH4_EMIT_TST_REG(rd) \
  SH4_EMIT_BYTE(0x2008 | ((rd & 0xF) << 8) | ((rd & 0xF) << 4))

#define SH4_EMIT_CMP_REG(rn, rm) \
  SH4_EMIT_BYTE(0x3000 | (((rn) & 0xF) << 8) | (((rm) & 0xF) << 4))

typedef enum {
  CONDITION_TRUE,
  CONDITION_FALSE,
  CONDITION_EQUAL,
  CONDITION_NOT_EQUAL
} condition_check_type;

extern u8 swi_hle_handle[256];
extern u32 idle_loop_target_pc;

#define block_prologue_size 0

u32 sh4_update_gba(u32 pc);
void sh4_indirect_branch_arm(u32 address);
void sh4_indirect_branch_thumb(u32 address);
void sh4_indirect_branch_dual(u32 address);
void sh4_step_debug(u32 pc);
void sh4_cheat_hook(void);
u32 function_cc execute_arm_translate(u32 cycles);

#define arm_process_cheats() \
  generate_function_call(sh4_cheat_hook)

#define thumb_process_cheats() \
  generate_function_call(sh4_cheat_hook)

#define generate_load_reg(ireg, reg_index) \
  SH4_EMIT_LOAD_REG(SH4_IREG(ireg), reg_index)

#define generate_store_reg(ireg, reg_index) \
  SH4_EMIT_STORE_REG(SH4_IREG(ireg), reg_index)

#define generate_load_imm(ireg, imm) \
  SH4_EMIT_LOAD_IMM(SH4_IREG(ireg), imm)

#define generate_load_pc(ireg, new_pc) \
  SH4_EMIT_LOAD_IMM(SH4_IREG(ireg), new_pc)

#define generate_mov(ireg_dest, ireg_src) \
  SH4_EMIT_MOV(SH4_IREG(ireg_dest), SH4_IREG(ireg_src))

#define generate_add(ireg_dest, ireg_src) \
  SH4_EMIT_ADD(SH4_IREG(ireg_dest), SH4_IREG(ireg_dest), SH4_IREG(ireg_src))

#define generate_sub(ireg_dest, ireg_src) \
  SH4_EMIT_SUB(SH4_IREG(ireg_dest), SH4_IREG(ireg_dest), SH4_IREG(ireg_src))

#define generate_or(ireg_dest, ireg_src) \
  SH4_EMIT_OR(SH4_IREG(ireg_dest), SH4_IREG(ireg_dest), SH4_IREG(ireg_src))

#define generate_xor(ireg_dest, ireg_src) \
  SH4_EMIT_XOR(SH4_IREG(ireg_dest), SH4_IREG(ireg_dest), SH4_IREG(ireg_src))

#define generate_and_imm(ireg, imm) \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r1, imm); \
    SH4_EMIT_AND(SH4_IREG(ireg), SH4_IREG(ireg), sh4_reg_r1); \
  } while(0)

#define generate_xor_imm(ireg, imm) \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r1, imm); \
    SH4_EMIT_XOR(SH4_IREG(ireg), SH4_IREG(ireg), sh4_reg_r1); \
  } while(0)

#define generate_add_imm(ireg, imm) \
  SH4_EMIT_ADDI(SH4_IREG(ireg), SH4_IREG(ireg), imm)

#define generate_sub_imm(ireg, imm) \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r1, imm); \
    SH4_EMIT_SUB(SH4_IREG(ireg), SH4_IREG(ireg), sh4_reg_r1); \
  } while(0)

#define generate_shift_left(ireg, imm_val) \
  do { u32 _sh = (imm_val); while(_sh--) SH4_EMIT_SHLL1(SH4_IREG(ireg)); } while(0)

#define generate_shift_right(ireg, imm_val) \
  do { u32 _sh = (imm_val); while(_sh--) SH4_EMIT_SHLR1(SH4_IREG(ireg)); } while(0)

#define generate_shift_right_arithmetic(ireg, imm_val) \
  do { u32 _sh = (imm_val); while(_sh--) SH4_EMIT_SHAR1(SH4_IREG(ireg)); } while(0)

#define generate_rotate_right(ireg, imm_val) \
  do { u32 _sh = (imm_val); while(_sh--) SH4_EMIT_ROTR1(SH4_IREG(ireg)); } while(0)

#define get_shift_imm() \
  u32 shift = (opcode >> 7) & 0x1F

#define generate_shift_reg(ireg, name, flags_op) \
  generate_load_reg_pc(ireg, rm, 12); \
  generate_load_reg(a1, ((opcode >> 8) & 0x0F)); \
  generate_function_call(execute_##name##_##flags_op##_reg); \
  generate_mov(ireg, rv)

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm) \
  do { \
    generate_mov(ireg_dest, ireg_src); \
    generate_add_imm(ireg_dest, imm); \
  } while(0)

#define generate_multiply(ireg) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_regs)

#define generate_multiply_s64(ireg) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_s64)

#define generate_multiply_u64(ireg) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_u64)

#define generate_multiply_s64_add(ireg_src, ireg_lo, ireg_hi) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_regs)

#define generate_multiply_u64_add(ireg_src, ireg_lo, ireg_hi) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_regs)

#define generate_function_call(function_location) \
  SH4_EMIT_FUNCTION_CALL(function_location)

#define generate_cycle_update() \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r1, cycle_count); \
    SH4_EMIT_SUB(REG_CYCLES, REG_CYCLES, sh4_reg_r1); \
    cycle_count = 0; \
  } while(0)

#define generate_cycle_update_force() generate_cycle_update()

#define generate_branch_patch_conditional(dest, offset) \
  *((u16 *)(dest)) = ((*((u16 *)(dest)) & 0xFF00) | \
   (SH4_RELATIVE_OFFSET((dest), (offset)) & 0xFF))

#define generate_branch_patch_unconditional(dest, offset) \
  do { \
    u16 _rel = SH4_RELATIVE_OFFSET((dest), (offset)) & 0x0FFF; \
    *((u16 *)(dest)) = (0xA000 | _rel); \
  } while(0)

#define generate_update_pc(new_pc) \
  SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc)

#define SH4_EMIT_RELOAD_CYCLES() \
  SH4_EMIT_MOV(REG_CYCLES, sh4_reg_r0)

#define generate_update_pc_reg() \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r4, pc); \
    SH4_EMIT_FUNCTION_CALL(sh4_update_gba); \
    SH4_EMIT_RELOAD_CYCLES(); \
  } while(0)

#define generate_branch_filler_true(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_TST_REG(SH4_IREG(ireg_dest)); \
    SH4_EMIT_BT_FILLER(writeback_location); \
  } while(0)

#define generate_branch_filler_false(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_TST_REG(SH4_IREG(ireg_dest)); \
    SH4_EMIT_BF_FILLER(writeback_location); \
  } while(0)

#define generate_branch_filler_equal(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_BF_FILLER(writeback_location); \
  } while(0)

#define generate_branch_filler_not_equal(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_BT_FILLER(writeback_location); \
  } while(0)

#define generate_conditional_branch(ireg_a, ireg_b, type, writeback_location) \
  generate_branch_filler_##type(ireg_a, ireg_b, writeback_location)

#define generate_branch_no_cycle_update(writeback_location, new_pc) \
  do { \
    u8 *_skip_update; \
    if(pc == idle_loop_target_pc) { \
      SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc); \
      SH4_EMIT_FUNCTION_CALL(sh4_update_gba); \
      SH4_EMIT_RELOAD_CYCLES(); \
      SH4_EMIT_BRA_FILLER(writeback_location); \
    } else { \
      SH4_EMIT_TST_REG(REG_CYCLES); \
      SH4_EMIT_BT_FILLER(_skip_update); \
      SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc); \
      SH4_EMIT_FUNCTION_CALL(sh4_update_gba); \
      SH4_EMIT_RELOAD_CYCLES(); \
      generate_branch_patch_conditional(_skip_update, translation_ptr); \
      SH4_EMIT_BRA_FILLER(writeback_location); \
    } \
  } while(0)

#define generate_branch_cycle_update(writeback_location, new_pc) \
  do { \
    generate_cycle_update(); \
    generate_branch_no_cycle_update(writeback_location, new_pc); \
  } while(0)

#define generate_branch() \
  generate_branch_cycle_update( \
   block_exits[block_exit_position].branch_source, \
   block_exits[block_exit_position].branch_target); \
  block_exit_position++

#define generate_indirect_branch_cycle_update(type) \
  SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_##type)

#define generate_indirect_branch_no_cycle_update(type) \
  SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_##type)

#define generate_block_prologue() \

#define generate_block_extra_vars_arm() \
  void generate_indirect_branch_arm() { \
    if(condition == 0x0E) \
      generate_indirect_branch_cycle_update(arm); \
    else \
      generate_indirect_branch_no_cycle_update(arm); \
  } \
  void generate_indirect_branch_dual() { \
    if(condition == 0x0E) \
      generate_indirect_branch_cycle_update(dual); \
    else \
      generate_indirect_branch_no_cycle_update(dual); \
  }

#define generate_block_extra_vars_thumb() \

#define translate_invalidate_dcache_region(cache_start, cache_end) \
  do { \
    sh4_invalidate_icache_region((u32)(cache_start), \
     (u32)((u8 *)(cache_end) - (u8 *)(cache_start)) + 0x100); \
  } while(0)

#define generate_load_reg_pc(ireg, reg_index, pc_offset) \
  if(reg_index == 15) \
    generate_load_pc(ireg, pc + pc_offset); \
  else \
    generate_load_reg(ireg, reg_index)

#define generate_store_reg_pc_no_flags(ireg, reg_index) \
  do { \
    generate_store_reg(ireg, reg_index); \
    if(reg_index == 15) { \
      SH4_EMIT_MOV(sh4_reg_r4, SH4_IREG(ireg)); \
      generate_indirect_branch_arm(); \
    } \
  } while(0)

#define generate_store_reg_pc_flags(ireg, reg_index) \
  do { \
    generate_store_reg(ireg, reg_index); \
    if(reg_index == 15) { \
      SH4_EMIT_MOV(sh4_reg_r4, SH4_IREG(ireg)); \
      SH4_EMIT_FUNCTION_CALL(execute_spsr_restore); \
      SH4_EMIT_MOV(sh4_reg_r4, sh4_reg_r0); \
      generate_indirect_branch_dual(); \
    } \
  } while(0)

#define generate_condition_eq(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_Z_FLAG); condition_check = CONDITION_TRUE

#define generate_condition_ne(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_Z_FLAG); condition_check = CONDITION_FALSE

#define generate_condition_cs(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_C_FLAG); condition_check = CONDITION_TRUE

#define generate_condition_cc(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_C_FLAG); condition_check = CONDITION_FALSE

#define generate_condition_mi(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_N_FLAG); condition_check = CONDITION_TRUE

#define generate_condition_pl(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_N_FLAG); condition_check = CONDITION_FALSE

#define generate_condition_vs(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_V_FLAG); condition_check = CONDITION_TRUE

#define generate_condition_vc(ireg_a, ireg_b) \
  generate_load_reg(ireg_a, REG_V_FLAG); condition_check = CONDITION_FALSE

#define generate_condition_hi(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_C_FLAG); \
    generate_xor_imm(ireg_a, 1); \
    generate_load_reg(ireg_b, REG_Z_FLAG); \
    generate_or(ireg_a, ireg_b); \
    condition_check = CONDITION_FALSE; \
  } while(0)

#define generate_condition_ls(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_C_FLAG); \
    generate_xor_imm(ireg_a, 1); \
    generate_load_reg(ireg_b, REG_Z_FLAG); \
    generate_or(ireg_a, ireg_b); \
    condition_check = CONDITION_TRUE; \
  } while(0)

#define generate_condition_ge(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_N_FLAG); \
    generate_load_reg(ireg_b, REG_V_FLAG); \
    condition_check = CONDITION_EQUAL; \
  } while(0)

#define generate_condition_lt(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_N_FLAG); \
    generate_load_reg(ireg_b, REG_V_FLAG); \
    condition_check = CONDITION_NOT_EQUAL; \
  } while(0)

#define generate_condition_gt(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_N_FLAG); \
    generate_load_reg(ireg_b, REG_V_FLAG); \
    generate_xor(ireg_b, ireg_a); \
    generate_load_reg(a0, REG_Z_FLAG); \
    generate_or(ireg_a, ireg_b); \
    condition_check = CONDITION_FALSE; \
  } while(0)

#define generate_condition_le(ireg_a, ireg_b) \
  do { \
    generate_load_reg(ireg_a, REG_N_FLAG); \
    generate_load_reg(ireg_b, REG_V_FLAG); \
    generate_xor(ireg_b, ireg_a); \
    generate_load_reg(a0, REG_Z_FLAG); \
    generate_or(ireg_a, ireg_b); \
    condition_check = CONDITION_TRUE; \
  } while(0)

#define generate_condition(ireg_a, ireg_b) \
  switch(condition) { \
    case 0x0: generate_condition_eq(ireg_a, ireg_b); break; \
    case 0x1: generate_condition_ne(ireg_a, ireg_b); break; \
    case 0x2: generate_condition_cs(ireg_a, ireg_b); break; \
    case 0x3: generate_condition_cc(ireg_a, ireg_b); break; \
    case 0x4: generate_condition_mi(ireg_a, ireg_b); break; \
    case 0x5: generate_condition_pl(ireg_a, ireg_b); break; \
    case 0x6: generate_condition_vs(ireg_a, ireg_b); break; \
    case 0x7: generate_condition_vc(ireg_a, ireg_b); break; \
    case 0x8: generate_condition_hi(ireg_a, ireg_b); break; \
    case 0x9: generate_condition_ls(ireg_a, ireg_b); break; \
    case 0xA: generate_condition_ge(ireg_a, ireg_b); break; \
    case 0xB: generate_condition_lt(ireg_a, ireg_b); break; \
    case 0xC: generate_condition_gt(ireg_a, ireg_b); break; \
    case 0xD: generate_condition_le(ireg_a, ireg_b); break; \
    default: break; \
  }

#define generate_conditional_branch_type(ireg_a, ireg_b) \
  switch(condition_check) { \
    case CONDITION_TRUE: generate_conditional_branch(ireg_a, ireg_b, true, backpatch_address); break; \
    case CONDITION_FALSE: generate_conditional_branch(ireg_a, ireg_b, false, backpatch_address); break; \
    case CONDITION_EQUAL: generate_conditional_branch(ireg_a, ireg_b, equal, backpatch_address); break; \
    case CONDITION_NOT_EQUAL: generate_conditional_branch(ireg_a, ireg_b, not_equal, backpatch_address); break; \
  }

#define arm_conditional_block_header() \
  generate_condition(a0, a1); \
  generate_conditional_branch_type(a0, a1)

#define generate_translation_gate(type) \
  do { \
    generate_update_pc(pc); \
    generate_indirect_branch_no_cycle_update(type); \
  } while(0)

#define generate_step_debug() \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r4, pc); \
    SH4_EMIT_FUNCTION_CALL(sh4_step_debug); \
  } while(0)

void swi_hle_div(void);

#define generate_swi_hle_handler(_swi_number) \
{ \
  u32 swi_number = _swi_number; \
  if(swi_hle_handle[swi_number]) \
  { \
    if(swi_number == 0x06) \
      generate_function_call(swi_hle_div); \
    break; \
  } \
}

#include "sh4_instr.inc"

#endif /* SH4_EMIT_H */
