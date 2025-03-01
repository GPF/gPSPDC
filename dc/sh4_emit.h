#ifndef SH4_EMIT_H
#define SH4_EMIT_H

#include "common.h"
#include "cpu.h"

typedef u16* translation_ptr_t;
extern translation_ptr_t translation_ptr;
void translate_invalidate_dcache(void);
void sh4_invalidate_icache_region(u32 addr, u32 size);

typedef enum {
  sh4_reg_r0  = 0,  // Temporary / Return value / Argument 0 (eax)
  sh4_reg_r1  = 1,  // Temporary / Argument 1 (edx)
  sh4_reg_r2  = 2,  // Temporary / Argument 2 (ecx)
  sh4_reg_r3  = 3,  // Temporary / Argument 3
  sh4_reg_r4  = 4,  // ARM R0 (static)
  sh4_reg_r5  = 5,  // ARM R1 (static)
  sh4_reg_r6  = 6,  // ARM R2 (static)
  sh4_reg_r7  = 7,  // ARM R3 (static)
  sh4_reg_r8  = 8,  // ARM R12 (static, often IP)
  sh4_reg_r9  = 9,  // ARM R13 (SP, static)
  sh4_reg_r10 = 10, // ARM R14 (LR, static)
  sh4_reg_r11 = 11, // ARM R15 (PC, static)
  sh4_reg_r12 = 12, // Base register (emulator state pointer, reg[], ebx)
  sh4_reg_r13 = 13, // Cycle counter (edi)
  sh4_reg_r14 = 14, // Scratch / Temporary (esi)
  sh4_reg_r15 = 15  // Stack pointer (SP)
} sh4_reg_number;

#define REG_BASE    sh4_reg_r12
#define REG_CYCLES  sh4_reg_r13
#define REG_TEMP    sh4_reg_r14
#define REG_A0      sh4_reg_r0
#define REG_A1      sh4_reg_r1
#define REG_A2      sh4_reg_r2
#define REG_A3      sh4_reg_r3
#define REG_RV      sh4_reg_r0
#define REG_S0      sh4_reg_r14

#define REG_R0      sh4_reg_r4
#define REG_R1      sh4_reg_r5
#define REG_R2      sh4_reg_r6
#define REG_R3      sh4_reg_r7
#define REG_R12     sh4_reg_r8
#define REG_SP      sh4_reg_r9
#define REG_LR      sh4_reg_r10
#define REG_PC      sh4_reg_r11

extern u32 arm_to_sh4_reg[];

// SH-4 Instruction Emission Macros
#define SH4_EMIT_BYTE(value) \
  *(translation_ptr++) = (value)

#define SH4_EMIT_DWORD(value) \
  *(translation_ptr++) = (value & 0xFFFF); \
  *(translation_ptr++) = ((value >> 16) & 0xFFFF)

#define SH4_EMIT_ADD(rd, rn, rm) \
  SH4_EMIT_BYTE(0x300C | ((rd & 0xF) << 8) | ((rn & 0xF) << 4) | (rm & 0xF))

#define SH4_EMIT_SUB(rd, rn, rm) \
  SH4_EMIT_BYTE(0x3008 | ((rd & 0xF) << 8) | ((rn & 0xF) << 4) | (rm & 0xF))

#define SH4_EMIT_MOV(rd, rm) \
  SH4_EMIT_BYTE(0x6003 | ((rd & 0xF) << 8) | ((rm & 0xF) << 4))

#define SH4_EMIT_MOVI(rd, imm) \
  SH4_EMIT_BYTE(0xE000 | ((rd & 0xF) << 8) | (imm & 0xFF))

#define SH4_EMIT_ADDI(rd, rn, imm) \
  SH4_EMIT_BYTE(0x7000 | ((rn & 0xF) << 8) | (imm & 0xFF)); \
  if ((rd) != (rn)) SH4_EMIT_MOV(rd, rn)

#define SH4_EMIT_LW(rd, rn, offset) \
  SH4_EMIT_BYTE(0x6006 | ((rd & 0xF) << 8) | ((rn & 0xF) << 4) | ((offset >> 2) & 0xF))

#define SH4_EMIT_SW(rd, rn, offset) \
  SH4_EMIT_BYTE(0x2006 | ((rd & 0xF) << 8) | ((rn & 0xF) << 4) | ((offset >> 2) & 0xF))

#define SH4_EMIT_CALL(target) \
  SH4_EMIT_BYTE(0xD000 | (((u32)(target) >> 2) & 0xFFF)); \
  SH4_EMIT_BYTE(0x0009)

#define SH4_EMIT_JMP(target_reg) \
  SH4_EMIT_BYTE(0x402B | ((target_reg & 0xF) << 8)); \
  SH4_EMIT_BYTE(0x0009)

#define SH4_EMIT_BRA(offset) \
  SH4_EMIT_BYTE(0xA000 | ((offset >> 1) & 0xFFF)); \
  SH4_EMIT_BYTE(0x0009)

#define SH4_EMIT_BT(offset) \
  SH4_EMIT_BYTE(0x8900 | ((offset >> 1) & 0xFF)); \
  SH4_EMIT_BYTE(0x0009)

#define SH4_EMIT_BF(offset) \
  SH4_EMIT_BYTE(0x8B00 | ((offset >> 1) & 0xFF)); \
  SH4_EMIT_BYTE(0x0009)

#define SH4_EMIT_NOP() \
  SH4_EMIT_BYTE(0x0009)

// Types and Defines from x86_emit.h
typedef enum {
  CONDITION_TRUE,
  CONDITION_FALSE,
  CONDITION_EQUAL,
  CONDITION_NOT_EQUAL
} condition_check_type;

extern u8 swi_hle_handle[256];
#define block_prologue_size 8 // No prologue for SH-4 yet, adjust if needed

// Function Declarations
u32 sh4_update_gba(u32 pc);
void sh4_indirect_branch_arm(u32 address);
void sh4_indirect_branch_thumb(u32 address);
void sh4_indirect_branch_dual(u32 address);

u32 function_cc sh4_execute_load_u8(u32 address);
u32 function_cc sh4_execute_load_u16(u32 address);
u32 function_cc sh4_execute_load_u32(u32 address);
s32 function_cc sh4_execute_load_s8(u32 address);
s32 function_cc sh4_execute_load_s16(u32 address);

void function_cc sh4_execute_store_u8(u32 address, u32 value);
void function_cc sh4_execute_store_u16(u32 address, u32 value);
void function_cc sh4_execute_store_u32(u32 address, u32 value);

u32 sh4_execute_aligned_load32(u32 address);
void sh4_execute_aligned_store32(u32 address, u32 value);

u32 function_cc execute_and(u32 rm, u32 rn);
u32 function_cc execute_ands(u32 rm, u32 rn);
u32 function_cc execute_eor(u32 rm, u32 rn);
u32 function_cc execute_eors(u32 rm, u32 rn);
u32 function_cc execute_sub(u32 rm, u32 rn);
u32 function_cc execute_subs(u32 rm, u32 rn);
u32 function_cc execute_rsb(u32 rm, u32 rn);
u32 function_cc execute_rsbs(u32 rm, u32 rn);
u32 function_cc execute_add(u32 rm, u32 rn);
u32 function_cc execute_adds(u32 rm, u32 rn);
u32 function_cc execute_adc(u32 rm, u32 rn);
u32 function_cc execute_adcs(u32 rm, u32 rn);
u32 function_cc execute_sbc(u32 rm, u32 rn);
u32 function_cc execute_sbcs(u32 rm, u32 rn);
u32 function_cc execute_rsc(u32 rm, u32 rn);
u32 function_cc execute_rscs(u32 rm, u32 rn);
u32 function_cc execute_orr(u32 rm, u32 rn);
u32 function_cc execute_orrs(u32 rm, u32 rn);
u32 function_cc execute_mov(u32 rm);
u32 function_cc execute_movs(u32 rm);
u32 function_cc execute_bic(u32 rm, u32 rn);
u32 function_cc execute_bics(u32 rm, u32 rn);
u32 function_cc execute_mvn(u32 rm);
u32 function_cc execute_mvns(u32 rm);
u32 function_cc execute_mul(u32 rm, u32 rs);
u32 function_cc execute_muls(u32 rm, u32 rs);
u32 function_cc execute_mul_long(u32 rm, u32 rs);
u32 function_cc execute_tst(u32 rm, u32 rn);
u32 function_cc execute_teq(u32 rm, u32 rn);
u32 function_cc execute_cmp(u32 rm, u32 rn);
u32 function_cc execute_cmn(u32 rm, u32 rn);
u32 function_cc execute_lsl_imm(u32 value, u32 shift);
u32 function_cc execute_lsr_imm(u32 value, u32 shift);
u32 function_cc execute_asr_imm(u32 value, u32 shift);
u32 function_cc execute_lsl_reg(u32 value, u32 shift);
u32 function_cc execute_lsr_reg(u32 value, u32 shift);
u32 function_cc execute_asr_reg(u32 value, u32 shift);
u32 function_cc execute_ror_reg(u32 value, u32 shift);
u32 function_cc execute_neg(u32 rm);

u32 sh4_execute_read_cpsr(void);
u32 sh4_execute_read_spsr(void);
void sh4_execute_swi(u32 pc);
u32 sh4_execute_spsr_restore(u32 address);
void sh4_execute_store_cpsr(u32 new_cpsr, u32 store_mask);
void sh4_execute_store_spsr(u32 new_spsr, u32 store_mask);

u32 sh4_execute_lsl_flags_reg(u32 value, u32 shift);
u32 sh4_execute_lsr_flags_reg(u32 value, u32 shift);
u32 sh4_execute_asr_flags_reg(u32 value, u32 shift);
u32 sh4_execute_ror_flags_reg(u32 value, u32 shift);

u32 function_cc sh4_execute_arm_translate(u32 cycles);
u32 function_cc sh4_execute_thumb_translate(u32 cycles);
void sh4_step_debug(u32 pc);

// Translation Macros
#define SH4_EMIT_LOAD_REG(ireg, reg_index) \
  SH4_EMIT_LW(ireg, REG_BASE, reg_index * 4)

#define SH4_EMIT_STORE_REG(ireg, reg_index) \
  SH4_EMIT_SW(ireg, REG_BASE, reg_index * 4)

#define SH4_EMIT_OR(rd, rn, rm) \
  SH4_EMIT_BYTE(0x300B | ((rd & 0xF) << 8) | ((rn & 0xF) << 4) | (rm & 0xF))

#define SH4_EMIT_LOAD_IMM(ireg, imm) \
  if ((imm & 0xFF) == imm) { \
    SH4_EMIT_MOVI(ireg, imm); \
  } else { \
    SH4_EMIT_MOVI(ireg, imm & 0xFF); \
    SH4_EMIT_MOVI(REG_TEMP, (imm >> 8) & 0xFF); \
    SH4_EMIT_ADDI(ireg, ireg, 8); \
    SH4_EMIT_OR(ireg, ireg, REG_TEMP); \
    if (imm > 0xFFFF) { \
      SH4_EMIT_MOVI(REG_TEMP, (imm >> 16) & 0xFF); \
      SH4_EMIT_ADDI(ireg, ireg, 8); \
      SH4_EMIT_OR(ireg, ireg, REG_TEMP); \
      SH4_EMIT_MOVI(REG_TEMP, (imm >> 24) & 0xFF); \
      SH4_EMIT_ADDI(ireg, ireg, 8); \
      SH4_EMIT_OR(ireg, ireg, REG_TEMP); \
    } \
  }

#define SH4_EMIT_FUNCTION_CALL(func) \
  SH4_EMIT_CALL(func)

#define SH4_EMIT_JMP_OFFSET(offset) \
  SH4_EMIT_BRA(SH4_RELATIVE_OFFSET(translation_ptr, offset))

#define SH4_RELATIVE_OFFSET(source, target) \
  (((u32)(target) - ((u32)(source) + 2)) >> 1)

// Translation Helpers
#define generate_load_reg(ireg, reg_index) \
  SH4_EMIT_LOAD_REG(ireg, reg_index)

#define generate_store_reg(ireg, reg_index) \
  SH4_EMIT_STORE_REG(ireg, reg_index)

#define generate_load_imm(ireg, imm) \
  SH4_EMIT_LOAD_IMM(ireg, imm)

#define generate_function_call(func) \
  SH4_EMIT_FUNCTION_CALL(func)

#define generate_indirect_branch_arm() \
  SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_arm)

#define generate_indirect_branch_thumb() \
  SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_thumb)

#define generate_indirect_branch_dual() \
  SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_dual)

#define generate_branch() \
  SH4_EMIT_JMP_OFFSET(block_exits[block_exit_position].branch_target); \
  block_exit_position++

#define generate_block_prologue() \
  /* No prologue for SH-4 yet, can add initialization here if needed */

#define arm_conditional_block_header() \
  /* Placeholder for SH-4 conditional block header */

  #define arm_bx() \
  do { \
    u32 rn = opcode & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_dual); \
  } while (0)

#define arm_b() \
  do { \
    s32 offset = ((s32)(opcode & 0xFFFFFF) << 8) >> 6; \
    branch_target = block_end_pc + 4 + offset; \
    block_exits[block_exit_position].branch_target = branch_target; \
    block_exits[block_exit_position].branch_source = translation_ptr; \
    SH4_EMIT_BRA(0); \
    block_exit_position++; \
  } while (0)

#define arm_bl() \
  do { \
    s32 offset = ((s32)(opcode & 0xFFFFFF) << 8) >> 6; \
    branch_target = block_end_pc + 4 + offset; \
    SH4_EMIT_MOVI(REG_LR, block_end_pc + 4); /* Set link register */ \
    block_exits[block_exit_position].branch_target = branch_target; \
    block_exits[block_exit_position].branch_source = translation_ptr; \
    SH4_EMIT_BRA(0); \
    block_exit_position++; \
  } while (0)

#define arm_swi() \
  do { \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_swi); \
  } while (0)

#define generate_branch_patch_conditional(backpatch_address, target) \
  SH4_EMIT_BRANCH_PATCH_CONDITIONAL(backpatch_address, target)

#define generate_branch_patch_unconditional(backpatch_address, target) \
  SH4_EMIT_BRANCH_PATCH_UNCONDITIONAL(backpatch_address, target)

#define generate_update_pc_reg() \
  SH4_EMIT_FUNCTION_CALL(sh4_update_gba)

#define generate_cycle_update() \
  SH4_EMIT_CYCLE_UPDATE()

#define generate_translation_gate(type) \
  SH4_EMIT_FUNCTION_CALL(sh4_execute_##type##_translate)

#define SH4_EMIT_CYCLE_UPDATE() \
  SH4_EMIT_SUB(REG_CYCLES, REG_CYCLES, cycle_count); \
  cycle_count = 0

#define SH4_EMIT_BRANCH_PATCH_CONDITIONAL(dest, offset) \
  *((u16 *)(dest)) = (SH4_RELATIVE_OFFSET(dest, offset) & 0xFF)

#define SH4_EMIT_BRANCH_PATCH_UNCONDITIONAL(dest, offset) \
  *((u16 *)(dest)) = (SH4_RELATIVE_OFFSET(dest, offset) & 0xFFF)

// Expanded Translation Macros from x86_emit.h
#define arm_data_proc(name, type, flags_op) \
  do { \
    u32 rn = (opcode >> 16) & 0xF; \
    u32 rd = (opcode >> 12) & 0xF; \
    u32 rm = opcode & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rm); \
    SH4_EMIT_LOAD_REG(REG_A1, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define arm_data_proc_unary(name, type, flags_op) \
  do { \
    u32 rd = (opcode >> 12) & 0xF; \
    u32 rm = opcode & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rm); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define arm_data_proc_test(name, type) \
  do { \
    u32 rn = (opcode >> 16) & 0xF; \
    u32 rm = opcode & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rm); \
    SH4_EMIT_LOAD_REG(REG_A1, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
  } while (0)

#define arm_multiply(add_op, flags) \
  do { \
    u32 rm = opcode & 0xF; \
    u32 rs = (opcode >> 8) & 0xF; \
    u32 rd = (opcode >> 16) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rm); \
    SH4_EMIT_LOAD_REG(REG_A1, rs); \
    SH4_EMIT_FUNCTION_CALL(execute_mul); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define arm_multiply_long(name, add_op, flags) \
  do { \
    u32 rm = opcode & 0xF; \
    u32 rs = (opcode >> 8) & 0xF; \
    u32 rdlo = (opcode >> 12) & 0xF; \
    u32 rdhi = (opcode >> 16) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rm); \
    SH4_EMIT_LOAD_REG(REG_A1, rs); \
    SH4_EMIT_FUNCTION_CALL(execute_mul_long); \
    SH4_EMIT_STORE_REG(REG_RV, rdlo); \
    SH4_EMIT_STORE_REG(REG_A1, rdhi); \
  } while (0)

#define arm_access_memory(access_type, direction, adjust_op, mem_type, offset_type) \
  do { \
    u32 rn = (opcode >> 16) & 0xF; \
    u32 rd = (opcode >> 12) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_##access_type##_##mem_type); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define arm_swap(type) \
  do { \
    u32 rn = (opcode >> 16) & 0xF; \
    u32 rd = (opcode >> 12) & 0xF; \
    u32 rm = opcode & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_load_##type); \
    SH4_EMIT_MOV(REG_S0, REG_RV); \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_LOAD_REG(REG_A1, rm); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_store_##type); \
    SH4_EMIT_STORE_REG(REG_S0, rd); \
  } while (0)

#define arm_psr(op_type, transfer_type, psr_reg) \
  do { \
    u32 rm = opcode & 0xF; \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_##transfer_type##_##psr_reg); \
    SH4_EMIT_STORE_REG(REG_RV, rm); \
  } while (0)

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn) \
  do { \
    u32 rd = opcode & 0x7; \
    u32 rs = (opcode >> 3) & 0x7; \
    u32 rn = (opcode >> 6) & 0x7; \
    SH4_EMIT_LOAD_REG(REG_A0, rs); \
    SH4_EMIT_LOAD_REG(REG_A1, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn) \
  do { \
    u32 rd = opcode & 0x7; \
    u32 rn = (opcode >> 3) & 0x7; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn) \
  do { \
    u32 rs = (opcode >> 3) & 0x7; \
    u32 rn = (opcode >> 6) & 0x7; \
    SH4_EMIT_LOAD_REG(REG_A0, rs); \
    SH4_EMIT_LOAD_REG(REG_A1, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
  } while (0)

#define thumb_shift(decode_type, op_type, value_type) \
  do { \
    u32 rd = opcode & 0x7; \
    u32 rs = (opcode >> 3) & 0x7; \
    SH4_EMIT_LOAD_REG(REG_A0, rs); \
    SH4_EMIT_FUNCTION_CALL(execute_##op_type##_##value_type); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define thumb_access_memory(access_type, op_type, reg_rd, reg_rb, reg_ro, address_type, offset, mem_type) \
  do { \
    u32 rd = (reg_rd); \
    u32 rb = (reg_rb); \
    SH4_EMIT_LOAD_REG(REG_A0, rb); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_##access_type##_##mem_type); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define arm_block_memory(access_type, pre_op, post_op, wb) \
  do { \
    u32 rn = (opcode >> 16) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_##access_type##_u32); \
    SH4_EMIT_STORE_REG(REG_RV, rn); \
  } while (0)

#define thumb_block_memory(access_type, pre_op, post_op, base_reg) \
  do { \
    u32 rb = (base_reg); \
    SH4_EMIT_LOAD_REG(REG_A0, rb); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_##access_type##_u32); \
    SH4_EMIT_STORE_REG(REG_RV, rb); \
  } while (0)

#define thumb_data_proc_hi(name) \
  do { \
    u32 rd = ((opcode >> 4) & 0x08) | (opcode & 0x7); \
    u32 rs = (opcode >> 3) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rs); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
    SH4_EMIT_STORE_REG(REG_RV, rd); \
  } while (0)

#define thumb_data_proc_test_hi(name) \
  do { \
    u32 rs = (opcode >> 3) & 0x7; \
    u32 rn = (opcode >> 6) & 0x7; \
    SH4_EMIT_LOAD_REG(REG_A0, rs); \
    SH4_EMIT_LOAD_REG(REG_A1, rn); \
    SH4_EMIT_FUNCTION_CALL(execute_##name); \
  } while (0)

#define thumb_conditional_branch(condition) \
  SH4_EMIT_BT(0)  // Placeholder, needs proper offset logic

#define thumb_load_pc(reg_num) \
  do { \
    u32 imm = (opcode & 0xFF) << 2; \
    SH4_EMIT_LOAD_IMM(REG_A0, block_end_pc + imm); \
    SH4_EMIT_STORE_REG(REG_RV, reg_num); \
  } while (0)

#define thumb_load_sp(reg_num) \
  do { \
    u32 imm = (opcode & 0xFF) << 2; \
    SH4_EMIT_LOAD_REG(REG_A0, REG_SP); \
    SH4_EMIT_ADDI(REG_A0, REG_A0, imm); \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_load_u32); \
    SH4_EMIT_STORE_REG(REG_RV, reg_num); \
  } while (0)

#define thumb_adjust_sp(imm_val) \
  do { \
    SH4_EMIT_LOAD_REG(REG_A0, REG_SP); \
    SH4_EMIT_ADDI(REG_A0, REG_A0, imm_val); \
    SH4_EMIT_STORE_REG(REG_A0, REG_SP); \
  } while (0)

#define thumb_b() \
  do { \
    u32 offset = (opcode & 0x7FF) << 1; \
    if (offset & 0x800) offset |= 0xFFFFF000; \
    branch_target = block_end_pc + 2 + offset; \
    block_exits[block_exit_position].branch_target = branch_target; \
    block_exits[block_exit_position].branch_source = translation_ptr; \
    SH4_EMIT_BRA(0); \
    block_exit_position++; \
  } while (0)

#define thumb_bl() \
  do { \
    u32 offset = ((last_opcode & 0x7FF) << 12) | ((opcode & 0x7FF) << 1); \
    if (offset & 0x400000) offset |= 0xFF800000; \
    branch_target = block_end_pc + 2 + offset; \
    block_exits[block_exit_position].branch_target = branch_target; \
    block_exits[block_exit_position].branch_source = translation_ptr; \
    SH4_EMIT_BRA(0); \
    block_exit_position++; \
  } while (0)

#define thumb_blh() \
  do { \
    SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_thumb); \
  } while (0)

#define thumb_swi() \
  do { \
    SH4_EMIT_FUNCTION_CALL(sh4_execute_swi); \
  } while (0)

#define thumb_bx() \
  do { \
    u32 rn = (opcode >> 3) & 0xF; \
    SH4_EMIT_LOAD_REG(REG_A0, rn); \
    SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_dual); \
  } while (0)

// Block Translation Variables
#define SH4_GENERATE_BLOCK_EXTRA_VARS() \
  u32 stored_pc = pc;

#define generate_block_extra_vars_arm SH4_GENERATE_BLOCK_EXTRA_VARS
#define generate_block_extra_vars_thumb SH4_GENERATE_BLOCK_EXTRA_VARS

#endif // SH4_EMIT_H