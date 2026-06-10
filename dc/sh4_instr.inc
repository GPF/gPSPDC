#define generate_shift_imm_lsl_no_flags(ireg)                                 \
  generate_load_reg_pc(ireg, rm, 8);                                          \
  if(shift != 0)                                                              \
  {                                                                           \
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

#define generate_shift_imm(ireg, name, flags_op)                              \
  get_shift_imm();                                                            \
  generate_shift_imm_##name##_##flags_op(ireg)                                \

#define generate_load_rm_sh(flags_op)                                         \
  switch((opcode >> 4) & 0x07)                                                \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSL reg */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsl, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR reg */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, lsr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x4:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR reg */                                                             \
    case 0x5:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, asr, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x6:                                                                 \
    {                                                                         \
      generate_shift_imm(a0, ror, flags_op);                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR reg */                                                             \
    case 0x7:                                                                 \
    {                                                                         \
      generate_shift_reg(a0, ror, flags_op);                                  \
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
    generate_load_pc(ireg, pc + pc_offset);                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

#define generate_store_reg_pc_no_flags(ireg, reg_index)                       \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a0, ireg);                                                   \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define generate_store_reg_pc_flags(ireg, reg_index)                          \
  generate_store_reg(ireg, reg_index);                                        \
  if(reg_index == 15)                                                         \
  {                                                                           \
    generate_mov(a0, ireg);                                                   \
    generate_function_call(execute_spsr_restore);                             \
    generate_mov(a0, rv);                                                     \
    generate_indirect_branch_dual();                                          \
  }                                                                           \

#define generate_condition_eq(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_Z_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_ne(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_Z_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_cs(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  condition_check = CONDITION_TRUE                                            \

#define generate_condition_cc(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_C_FLAG);                                      \
  condition_check = CONDITION_FALSE                                           \

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
  generate_load_reg(a0, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_FALSE                                           \

#define generate_condition_le(ireg_a, ireg_b)                                 \
  generate_load_reg(ireg_a, REG_N_FLAG);                                      \
  generate_load_reg(ireg_b, REG_V_FLAG);                                      \
  generate_xor(ireg_b, ireg_a);                                               \
  generate_load_reg(a0, REG_Z_FLAG);                                          \
  generate_or(ireg_a, ireg_b);                                                \
  condition_check = CONDITION_TRUE                                            \


#define generate_condition(ireg_a, ireg_b)                                    \
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
  }                                                                           \

#define generate_conditional_branch_type(ireg_a, ireg_b)                      \
  switch(condition_check)                                                     \
  {                                                                           \
    case CONDITION_TRUE:                                                      \
      generate_conditional_branch(ireg_a, ireg_b, true, backpatch_address);   \
      break;                                                                  \
                                                                              \
    case CONDITION_FALSE:                                                     \
      generate_conditional_branch(ireg_a, ireg_b, false, backpatch_address);  \
      break;                                                                  \
                                                                              \
    case CONDITION_EQUAL:                                                     \
      generate_conditional_branch(ireg_a, ireg_b, equal, backpatch_address);  \
      break;                                                                  \
                                                                              \
    case CONDITION_NOT_EQUAL:                                                 \
      generate_conditional_branch(ireg_a, ireg_b, not_equal,                  \
       backpatch_address);                                                    \
      break;                                                                  \
  }                                                                           \

#define generate_branch()                                                     \
{                                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++;                                                      \
}                                                                             \

#define rm_op_reg rm
#define rm_op_imm imm

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
  generate_load_rm_sh(no_flags)                                               \

#define arm_data_proc_imm()                                                   \
  arm_decode_data_proc_imm();                                                 \
  generate_load_imm(a0, imm)                                                  \

#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_##flags_op(rv, rd);                                   \
}                                                                             \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_load_reg_pc(a1, rn, 8);                                            \
  generate_function_call(execute_##name);                                     \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_function_call(execute_##name);                                     \
  generate_store_reg_pc_##flags_op(rv, rd);                                   \
}                                                                             \

#define arm_data_proc_mov(type)                                               \
{                                                                             \
  arm_data_proc_##type();                                                     \
  generate_store_reg_pc_no_flags(a0, rd);                                     \
}                                                                             \

#define arm_multiply_flags_yes()                                              \
  generate_function_call(execute_mul_flags)                                   \

#define arm_multiply_flags_no(_dest)                                          \

#define arm_multiply_add_no()                                                 \

#define arm_multiply_add_yes()                                                \
  generate_load_reg(a1, rn);                                                  \
  generate_add(a0, a1)                                                        \

#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply();                                                      \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  generate_multiply(a1);                                                      \
  arm_multiply_add_##add_op();                                                \
  generate_store_reg(a0, rd);                                                 \
  arm_multiply_flags_##flags();                                               \
}                                                                             \

#define arm_multiply_long_flags_yes()                                         \
  generate_function_call(execute_mul_long_flags)                              \

#define arm_multiply_long_flags_no(_dest)                                     \

#define arm_multiply_long_add_yes(name)                                       \
  generate_load_reg(a2, rdlo);                                                \
  generate_load_reg(s0, rdhi);                                                \
  generate_multiply_##name(a1, a2, s0)                                        \

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name(a1)                                                \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  generate_load_reg(a0, rm);                                                  \
  generate_load_reg(a1, rs);                                                  \
  arm_multiply_long_add_##add_op(name);                                       \
  generate_store_reg(a0, rdlo);                                               \
  generate_store_reg(a1, rdhi);                                               \
  arm_multiply_long_flags_##flags();                                          \
}                                                                             \

#define arm_psr_read(op_type, psr_reg)                                        \
  generate_function_call(execute_read_##psr_reg);                             \
  generate_store_reg(rv, rd)                                                  \

#define arm_psr_load_new_reg()                                                \
  generate_load_reg(a0, rm)                                                   \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(a0, imm)                                                  \

#define arm_psr_store_cpsr_post()                                             \
  do {                                                                        \
    u8 *_skip_irq;                                                            \
    SH4_EMIT_TST_REG(sh4_reg_r0);                                             \
    SH4_EMIT_BF_FILLER(_skip_irq);                                            \
    SH4_EMIT_MOV(sh4_reg_r4, sh4_reg_r0);                                     \
    generate_indirect_branch_arm();                                           \
    generate_branch_patch_conditional(_skip_irq, translation_ptr);            \
  } while(0)                                                                  \

#define arm_psr_store_finish_cpsr()                                           \
  generate_load_pc(a2, pc);                                                   \
  generate_function_call(execute_store_cpsr);                                 \
  arm_psr_store_cpsr_post()                                                   \

#define arm_psr_store_finish_spsr()                                           \
  generate_function_call(execute_store_spsr)                                  \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  generate_load_imm(a1, psr_masks[psr_field]);                                \
  arm_psr_store_finish_##psr_reg()                                            \

#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##op_type();                                                 \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

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


#define arm_access_memory_load(mem_type)                                      \
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg_pc_no_flags(rv, rd)                                      \

#define arm_access_memory_store(mem_type)                                     \
  generate_load_reg_pc(a1, rd, 12);                                           \
  generate_load_pc(a2, (pc + 4));                                             \
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
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op(a0, a1)                                            \

#define arm_access_memory_reg_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_reg_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_reg_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op(a0, a1);                                           \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op(a0, a1)                                           \

#define arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op)              \
  generate_load_reg_pc(a0, rn, 8);                                            \
  generate_##adjust_dir_op##_imm(a0, offset)                                  \

#define arm_access_memory_imm_pre_wb(adjust_dir_op, reverse_dir_op)           \
  arm_access_memory_imm_pre(adjust_dir_op, reverse_dir_op);                   \
  generate_store_reg(a0, rn)                                                  \

#define arm_access_memory_imm_post(adjust_dir_op, reverse_dir_op)             \
  generate_load_reg(a0, rn);                                                  \
  generate_##adjust_dir_op##_imm(a0, offset);                                 \
  generate_store_reg(a0, rn);                                                 \
  generate_##reverse_dir_op##_imm(a0, offset)                                 \


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
  generate_add_imm(s0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_preadjust_up()                                      \
  generate_add_imm(s0, 4)                                                     \

#define arm_block_address_preadjust_down_full()                               \
  generate_sub_imm(s0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_preadjust_down()                                    \
  generate_sub_imm(s0, ((word_bit_count(reg_list) * 4) - 4))                  \

#define arm_block_address_preadjust_no()                                      \

#define arm_block_address_postadjust_no()                                     \

#define arm_block_address_postadjust_up()                                     \
  generate_add_imm(a0, (word_bit_count(reg_list) * 4))                        \

#define arm_block_address_postadjust_down()                                   \
  generate_sub_imm(a0, (word_bit_count(reg_list) * 4))                        \

#define sprint_no(access_type, pre_op, post_op, wb)                           \

#define sprint_yes(access_type, pre_op, post_op, wb)                          \
  /* printf("sbit on %s %s %s %s\n", #access_type, #pre_op, #post_op, #wb)*/   \

#define arm_block_memory_branch_pc_load()                                     \
  if(opcode & 0x8000)                                                         \
  {                                                                           \
    generate_mov(a0, rv);                                                     \
    generate_indirect_branch_arm();                                           \
  }                                                                           \

#define arm_block_memory_branch_pc_store()                                    \

#define arm_block_memory(access_type, pre_op, post_op, s_bit)                 \
{                                                                             \
  generate_load_imm(a0, opcode);                                              \
  generate_load_pc(a1, pc);                                                   \
  generate_function_call(execute_arm_block_memory);                           \
  arm_block_memory_branch_pc_##access_type();                                 \
}                                                                             \

#define arm_swap(type)                                                        \
{                                                                             \
  arm_decode_swap();                                                          \
  generate_load_reg(a0, rn);                                                  \
  generate_function_call(execute_load_##type);                                \
  generate_mov(s0, rv);                                                       \
  generate_load_reg(a0, rn);                                                  \
  generate_load_reg(a1, rm);                                                  \
  generate_load_pc(a2, (pc + 4));                                             \
  generate_function_call(execute_store_##type);                               \
  generate_store_reg(s0, rd);                                                 \
}                                                                             \

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
  do {                                                                        \
    generate_store_reg(ireg, rd);                                             \
    if(rd == 15)                                                              \
    {                                                                         \
      SH4_EMIT_MOV(sh4_reg_r4, SH4_IREG(ireg));                               \
      generate_indirect_branch_cycle_update(thumb);                           \
    }                                                                         \
  } while(0)

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
  generate_load_pc(a0, (((pc & ~2) + 4) + (imm * 4)));                        \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (imm * 4));                                            \
  generate_store_reg(a0, _rd);                                                \
}                                                                             \

#define thumb_adjust_sp(value)                                                \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  generate_load_reg(a0, 13);                                                  \
  generate_add_imm(a0, (value));                                              \
  generate_store_reg(a0, 13);                                                 \
}                                                                             \

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

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
  generate_function_call(execute_load_##mem_type);                            \
  generate_store_reg(rv, reg_rd)                                              \

#define thumb_access_memory_store(mem_type, reg_rd)                           \
  generate_load_reg(a1, reg_rd);                                              \
  generate_load_pc(a2, (pc + 2));                                             \
  generate_function_call(execute_store_##mem_type)                            \

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
  generate_add_imm(s0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_down()                                  \
  generate_sub_imm(s0, (bit_count[reg_list] * 4))                             \

#define thumb_block_address_preadjust_push_lr()                               \
  generate_sub_imm(s0, ((bit_count[reg_list] + 1) * 4))                       \

#define thumb_block_address_preadjust_no()                                    \

#define thumb_block_address_postadjust_no(base_reg)                           \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_address_postadjust_up(base_reg)                           \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_down(base_reg)                         \
  generate_mov(a0, s0);                                                       \
  generate_sub_imm(a0, (bit_count[reg_list] * 4));                            \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_pop_pc(base_reg)                       \
  generate_add_reg_reg_imm(a0, s0, ((bit_count[reg_list] + 1) * 4));          \
  generate_store_reg(a0, base_reg)                                            \

#define thumb_block_address_postadjust_push_lr(base_reg)                      \
  generate_store_reg(s0, base_reg)                                            \

#define thumb_block_memory_extra_no()                                         \

#define thumb_block_memory_extra_up()                                         \

#define thumb_block_memory_extra_down()                                       \

#define thumb_block_memory_extra_pop_pc()                                     \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
  generate_function_call(execute_aligned_load32);                             \
  generate_store_reg(rv, REG_PC);                                             \
  generate_mov(a0, rv);                                                       \
  generate_indirect_branch_cycle_update(thumb)                                \

#define thumb_block_memory_extra_push_lr(base_reg)                            \
  generate_add_reg_reg_imm(a0, s0, (bit_count[reg_list] * 4));                \
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
  generate_load_reg(a1, i);                                                   \
  generate_load_pc(a2, (pc + 2));                                             \
  generate_function_call(execute_store_u32)                                   \

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
    {                                                                         \
      generate_add_reg_reg_imm(a0, s0, offset);                               \
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


#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  condition_check_type condition_check;                                       \
  generate_cycle_update();                                                    \
  generate_condition_##condition(a0, a1);                                     \
  generate_conditional_branch_type(a0, a1);                                   \
  generate_branch_no_cycle_update(                                            \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \

#define flags_vars(src_a, src_b)                                              \
  u32 dest;                                                                   \
  const u32 _sa = src_a;                                                      \
  const u32 _sb = src_b                                                       \

#define data_proc_generate_logic_function(name, expr)                         \
\
                                                                              \
\

#define data_proc_generate_logic_unary_function(name, expr)                   \
\
                                                                              \
\


#define data_proc_generate_sub_function(name, src_a, src_b)                   \
\
                                                                              \
\

#define data_proc_generate_add_function(name, src_a, src_b)                   \
\
                                                                              \
\

#define data_proc_generate_sub_test_function(name, src_a, src_b)              \
\

#define data_proc_generate_add_test_function(name, src_a, src_b)              \
\

#define data_proc_generate_logic_test_function(name, expr)                    \
\

\

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
data_proc_generate_sub_function(sbc, rn - rm, (reg[REG_C_FLAG] ^ 1));
data_proc_generate_sub_function(rsc, rm + reg[REG_C_FLAG] - 1, rn);
data_proc_generate_add_function(add, rn, rm);
data_proc_generate_add_function(adc, rn, rm + reg[REG_C_FLAG]);

data_proc_generate_logic_test_function(tst, rn & rm);
data_proc_generate_logic_test_function(teq, rn ^ rm);
data_proc_generate_sub_test_function(cmp, rn, rm);
data_proc_generate_add_test_function(cmn, rn, rm);

#define arm_conditional_block_header()                                        \
  generate_condition(a0, a1);                                                 \
  generate_conditional_branch_type(a0, a1);                                   \

#define arm_b()                                                               \
  generate_branch()                                                           \

#define arm_bl()                                                              \
  generate_update_pc((pc + 4));                                               \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch()                                                           \

#define arm_bx()                                                              \
  arm_decode_branchx();                                                       \
  generate_load_reg(a0, rn);                                                  \
  generate_indirect_branch_dual();                                            \

#define arm_swi()                                                             \
  generate_swi_hle_handler((opcode >> 16) & 0xFF);                            \
  generate_load_imm(a0, ((opcode >> 16) & 0xFF));                             \
  generate_load_imm(a1, (pc + 4));                                            \
  generate_load_imm(a2, 0);                                                   \
  generate_function_call(sh4_trace_swi);                                      \
  generate_update_pc((pc + 4));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch()                                                           \

#define thumb_b()                                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

#define thumb_bl()                                                            \
  generate_update_pc(((pc + 2) | 0x01));                                      \
  generate_store_reg(a0, REG_LR);                                             \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

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
  generate_load_imm(a0, (opcode & 0xFF));                                     \
  generate_load_imm(a1, (pc + 2));                                            \
  generate_load_imm(a2, 1);                                                   \
  generate_function_call(sh4_trace_swi);                                      \
  generate_update_pc((pc + 2));                                               \
  generate_function_call(execute_swi);                                        \
  generate_branch_cycle_update(                                               \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  block_exit_position++                                                       \

