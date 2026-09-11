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
#define sh4_reg_a3 sh4_reg_r7
#define sh4_reg_rv sh4_reg_r0
#define sh4_reg_s0 sh4_reg_r14

#define SH4_IREG(ireg) sh4_reg_##ireg

#define SH4_EMIT_BYTE(value) \
  *(translation_ptr++) = (value)

#define SH4_EMIT_NOP() \
  SH4_EMIT_BYTE(0x0009)

static inline s32 sh4_relative_offset_words(const void *source,
 const void *target)
{
  return (s32)(((const u8 *)target - ((const u8 *)source + 4)) >> 1);
}

static inline u32 sh4_branch12_in_range(const void *source,
 const void *target)
{
  s32 offset = sh4_relative_offset_words(source, target);
  return (offset >= -2048) && (offset <= 2047);
}

#define SH4_RELATIVE_OFFSET(source, target) \
  sh4_relative_offset_words((source), (target))

/* A long-branch slot starts with mov.l @(1,pc),r1 / jmp @r1 / nop and is
   followed by a 4-byte literal; the literal lands at +8 bytes when the slot
   starts 4-aligned (one alignment nop) and at +6 bytes otherwise. */
static inline u32 *sh4_long_branch_literal(void *slot)
{
  u8 *d = (u8 *)slot;

  if((((u32)(unsigned long)d + 6) & 3) != 0)
    return (u32 *)(d + 8);

  return (u32 *)(d + 6);
}

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
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_BYTE(0xA000); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_BT_FILLER(writeback_location) \
  do { \
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_BYTE(0x8900); \
    SH4_EMIT_NOP(); \
  } while(0)

/* Far-capable conditional skip.  A lone bt/bf only reaches +/-254 bytes, but
   the ARM per-condition block header skips an entire run of same-condition
   instructions, whose emitted body easily exceeds that.  Emit a short bt/bf
   that hops over a 12-bit bra (reach +/-4 KB) so the skip target can be
   patched anywhere in the block.  writeback_location points at the bra, which
   generate_branch_patch_conditional fills in.
     SH4_EMIT_COND_SKIP_T: take the bra (skip) when T == 1
     SH4_EMIT_COND_SKIP_F: take the bra (skip) when T == 0 */
#define SH4_EMIT_COND_SKIP_T(writeback_location) \
  do { \
    SH4_EMIT_BYTE(0x8B01); /* bf .+6: when T==0 fall through past the bra */ \
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_BYTE(0xA000); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_COND_SKIP_F(writeback_location) \
  do { \
    SH4_EMIT_BYTE(0x8901); /* bt .+6: when T==1 fall through past the bra */ \
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_BYTE(0xA000); \
    SH4_EMIT_NOP(); \
  } while(0)

/* Fills in the 8-bit displacement of a bt/bf emitted by a *_FILLER macro. */
static inline void sh4_patch_cond_hop(void *hop, const void *target)
{
  u16 *hop_word = (u16 *)hop;

  *hop_word = (u16)((*hop_word & 0xFF00) |
   (sh4_relative_offset_words(hop, target) & 0xFF));
}

/* Far-capable conditional skips for spans that may exceed the 12-bit bra:
   hop over an absolute-jump slot with the opposite-polarity bt/bf, so the
   skip itself has unlimited reach.  The writeback location points at the
   veneer, which generate_branch_patch_conditional recognizes by shape. */
#define SH4_EMIT_COND_SKIP_T_FAR(writeback_location) \
  do { \
    u8 *_cond_hop; \
    SH4_EMIT_BF_FILLER(_cond_hop); /* T==0: hop past the veneer */ \
    SH4_EMIT_LONG_BRANCH_FILLER(writeback_location); \
    sh4_patch_cond_hop(_cond_hop, translation_ptr); \
  } while(0)

#define SH4_EMIT_COND_SKIP_F_FAR(writeback_location) \
  do { \
    u8 *_cond_hop; \
    SH4_EMIT_BT_FILLER(_cond_hop); /* T==1: hop past the veneer */ \
    SH4_EMIT_LONG_BRANCH_FILLER(writeback_location); \
    sh4_patch_cond_hop(_cond_hop, translation_ptr); \
  } while(0)

#define SH4_EMIT_BF_FILLER(writeback_location) \
  do { \
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_BYTE(0x8B00); \
    SH4_EMIT_NOP(); \
  } while(0)

#define SH4_EMIT_LOAD_U8(rd, imm) \
  do { \
    u32 _u8 = (u32)(imm) & 0xFF; \
    SH4_EMIT_MOVI((rd), _u8); \
    if(_u8 & 0x80) SH4_EMIT_EXTU_B((rd), (rd)); \
  } while(0)

#define SH4_EMIT_MOVL_PC(rd, disp) \
  SH4_EMIT_BYTE(0xD000 | (((rd) & 0xF) << 8) | ((disp) & 0xFF))

#define SH4_EMIT_ABSOLUTE_JUMP_VENEER(target, literal_location) \
  do { \
    u32 _veneer_start = (u32)translation_ptr; \
    SH4_EMIT_MOVL_PC(sh4_reg_r1, 1); \
    SH4_EMIT_JMP(sh4_reg_r1); \
    if(((_veneer_start + 6) & 3) != 0) SH4_EMIT_NOP(); \
    (literal_location) = (u32 *)translation_ptr; \
    *((u32 *)translation_ptr) = (u32)(target); \
    translation_ptr += 2; \
  } while(0)

/* Block exits are emitted as fixed-shape absolute-jump slots because the
   12-bit bra displacement only reaches +/-4KB, while patch targets (internal
   loop heads in large blocks, linked external blocks) can be much further
   away.  In-range patches rewrite the slot to bra/nop; out-of-range patches
   keep the veneer instructions and just fill in the literal. */
#define SH4_EMIT_LONG_BRANCH_FILLER(writeback_location) \
  do { \
    u32 *_filler_literal; \
    (writeback_location) = (u8 *)translation_ptr; \
    SH4_EMIT_ABSOLUTE_JUMP_VENEER(0, _filler_literal); \
    (void)_filler_literal; \
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
      u32 _imm_shift = 24; \
      /* Skip leading zero bytes at translation time, not at runtime. */ \
      while(_imm_shift && ((_imm >> _imm_shift) == 0)) _imm_shift -= 8; \
      SH4_EMIT_LOAD_U8((rd), _imm >> _imm_shift); \
      while(_imm_shift) { \
        _imm_shift -= 8; \
        SH4_EMIT_SHLL8((rd)); \
        SH4_EMIT_ADD_UNSIGNED_BYTE((rd), _imm >> _imm_shift); \
      } \
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

#define SH4_EMIT_CMP_PZ(rn) \
  SH4_EMIT_BYTE(0x4011 | (((rn) & 0xF) << 8))

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
void sh4_indirect_branch_arm(u32 address, u32 cycles) __attribute__((noreturn));
void sh4_indirect_branch_thumb(u32 address, u32 cycles) __attribute__((noreturn));
void sh4_indirect_branch_dual(u32 address, u32 cycles) __attribute__((noreturn));
void sh4_step_debug(u32 pc, u32 cycles);
void sh4_cheat_hook(void);
void sh4_trace_swi(u32 swi_number, u32 pc, u32 thumb);
void sh4_trace_emit_update_pc(u32 new_pc, u32 source_pc, u32 opcode);
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
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_regs_s64)

#define generate_multiply_u64_add(ireg_src, ireg_lo, ireg_hi) \
  SH4_EMIT_FUNCTION_CALL(execute_mul_long_regs_u64)

#define generate_function_call(function_location) \
  SH4_EMIT_FUNCTION_CALL(function_location)

/* Store helpers take the live cycle counter (r13) as their fourth argument
   so SMC flushes and hardware alerts can re-enter translated code through
   the stack-resetting dispatcher. */
#define generate_store_call(mem_type) \
  do { \
    SH4_EMIT_MOV(sh4_reg_r7, REG_CYCLES); \
    generate_function_call(execute_store_##mem_type); \
  } while(0)

#ifdef GPSP_DC_RUNTIME_TRACE
#define generate_swi_trace(swi_num, return_pc, is_thumb) \
  do { \
    generate_load_imm(a0, (swi_num)); \
    generate_load_imm(a1, (return_pc)); \
    generate_load_imm(a2, (is_thumb)); \
    generate_function_call(sh4_trace_swi); \
  } while(0)
#else
#define generate_swi_trace(swi_num, return_pc, is_thumb)
#endif

#define generate_cycle_update() \
  do { \
    if(cycle_count != 0) { \
      SH4_EMIT_LOAD_IMM(sh4_reg_r1, cycle_count); \
      SH4_EMIT_SUB(REG_CYCLES, REG_CYCLES, sh4_reg_r1); \
      cycle_count = 0; \
    } \
  } while(0)

#define generate_cycle_update_force() generate_cycle_update()

void gpsp_dynarec_fatal_error(const char *detail);

/* Conditional skips reserve either a 12-bit bra (SH4_EMIT_COND_SKIP_*) or,
   for long conditional runs, a bt/bf hop over an absolute-jump slot
   (SH4_EMIT_COND_SKIP_*_FAR).  Dispatch on the slot's first word: a bra
   filler starts 0xAxxx, a veneer starts with mov.l @(disp,pc) (0xDxxx).
   A bra slot that cannot reach its target means the translate-time run
   estimate was wrong; fail loudly instead of emitting a corrupt branch. */
#define generate_branch_patch_conditional(dest, offset) \
  do { \
    if((*((u16 *)(dest)) & 0xF000) == 0xA000) { \
      if(!sh4_branch12_in_range((dest), (offset))) \
        gpsp_dynarec_fatal_error("conditional skip exceeds bra range"); \
      generate_branch_patch_unconditional_direct((dest), (offset)); \
    } else { \
      generate_branch_patch_unconditional((dest), (offset)); \
    } \
  } while(0)

#define generate_branch_patch_unconditional_direct(dest, offset) \
  do { \
    u16 _rel = SH4_RELATIVE_OFFSET((dest), (offset)) & 0x0FFF; \
    *((u16 *)(dest)) = (0xA000 | _rel); \
  } while(0)

/* Patches a SH4_EMIT_LONG_BRANCH_FILLER slot.  Near targets become
   bra/nop (the bra delay slot must not keep the veneer's jmp); far targets
   keep the veneer and receive the absolute address in its literal. */
#define generate_branch_patch_unconditional(dest, offset) \
  do { \
    if(sh4_branch12_in_range((dest), (offset))) { \
      generate_branch_patch_unconditional_direct((dest), (offset)); \
      ((u16 *)(dest))[1] = 0x0009; \
    } else { \
      *sh4_long_branch_literal(dest) = (u32)(offset); \
    } \
  } while(0)

#define generate_update_pc(new_pc) \
  do { \
    sh4_trace_emit_update_pc((new_pc), pc, opcode); \
    SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc); \
  } while(0)

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
    SH4_EMIT_COND_SKIP_T(writeback_location); \
  } while(0)

#define generate_branch_filler_false(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_TST_REG(SH4_IREG(ireg_dest)); \
    SH4_EMIT_COND_SKIP_F(writeback_location); \
  } while(0)

#define generate_branch_filler_equal(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_COND_SKIP_F(writeback_location); \
  } while(0)

#define generate_branch_filler_not_equal(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_COND_SKIP_T(writeback_location); \
  } while(0)

#define generate_conditional_branch(ireg_a, ireg_b, type, writeback_location) \
  generate_branch_filler_##type(ireg_a, ireg_b, writeback_location)

/* Far variants for conditional runs whose emitted body may exceed the
   12-bit bra reach (see arm_conditional_block_header). */
#define generate_branch_filler_true_far(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_TST_REG(SH4_IREG(ireg_dest)); \
    SH4_EMIT_COND_SKIP_T_FAR(writeback_location); \
  } while(0)

#define generate_branch_filler_false_far(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_TST_REG(SH4_IREG(ireg_dest)); \
    SH4_EMIT_COND_SKIP_F_FAR(writeback_location); \
  } while(0)

#define generate_branch_filler_equal_far(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_COND_SKIP_F_FAR(writeback_location); \
  } while(0)

#define generate_branch_filler_not_equal_far(ireg_dest, ireg_src, writeback_location) \
  do { \
    SH4_EMIT_CMP_REG(SH4_IREG(ireg_dest), SH4_IREG(ireg_src)); \
    SH4_EMIT_COND_SKIP_T_FAR(writeback_location); \
  } while(0)

#define generate_conditional_branch_far(ireg_a, ireg_b, type, writeback_location) \
  generate_branch_filler_##type##_far(ireg_a, ireg_b, writeback_location)

#define generate_branch_no_cycle_update(writeback_location, new_pc) \
  do { \
    u8 *_skip_update; \
    if(pc == idle_loop_target_pc) { \
      SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc); \
      SH4_EMIT_FUNCTION_CALL(sh4_update_gba); \
      SH4_EMIT_RELOAD_CYCLES(); \
      SH4_EMIT_LONG_BRANCH_FILLER(writeback_location); \
    } else { \
      SH4_EMIT_CMP_PZ(REG_CYCLES); \
      SH4_EMIT_COND_SKIP_T(_skip_update); \
      SH4_EMIT_LOAD_IMM(sh4_reg_r4, new_pc); \
      SH4_EMIT_FUNCTION_CALL(sh4_update_gba); \
      SH4_EMIT_RELOAD_CYCLES(); \
      generate_branch_patch_conditional(_skip_update, translation_ptr); \
      SH4_EMIT_LONG_BRANCH_FILLER(writeback_location); \
    } \
  } while(0)

#define generate_branch_cycle_update(writeback_location, new_pc) \
  do { \
    generate_cycle_update(); \
    generate_branch_no_cycle_update(writeback_location, new_pc); \
  } while(0)

/* The live SH-4 cycle counter (r13) is passed explicitly so the dispatch
   stub can re-enter the next block through the stack-resetting trampoline
   instead of nesting C call frames that never unwind. */
#define generate_indirect_branch_cycle_update(type) \
  do { \
    SH4_EMIT_MOV(sh4_reg_r5, REG_CYCLES); \
    SH4_EMIT_FUNCTION_CALL(sh4_indirect_branch_##type); \
  } while(0)

#define generate_indirect_branch_no_cycle_update(type) \
  generate_indirect_branch_cycle_update(type)

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

/* generate_load_reg_pc, generate_store_reg_pc_*, the generate_condition_*
   family, generate_conditional_branch_type, arm_conditional_block_header,
   and generate_branch live in sh4_instr.inc (included below), mirroring the
   x86 backend layout.  Keeping second copies here only produced macro
   redefinition warnings; the sh4_instr.inc definitions always won. */

#define generate_translation_gate(type) \
  do { \
    generate_update_pc(pc); \
    generate_indirect_branch_no_cycle_update(type); \
  } while(0)

#define generate_step_debug() \
  do { \
    SH4_EMIT_LOAD_IMM(sh4_reg_r4, pc); \
    SH4_EMIT_MOV(sh4_reg_r5, REG_CYCLES); \
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
