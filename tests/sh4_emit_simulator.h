/* Deliberately small test interpreter for the emitter sequences below.
   Unsupported instructions, invalid targets, and non-NOP delay slots fail.
   It is not an SH-4 CPU emulator or a timing model. */
static int simulate_emission(u32 regs[16], u32 t)
{
  u32 base = (u32)(uintptr_t)code_buffer;
  u32 pc = base;
  u32 bytes = (u32)((u8 *)translation_ptr - (u8 *)code_buffer);
  unsigned steps;
  for(steps = 0; steps < 256; steps++)
  {
    u32 offset = pc - base, next = pc + 2;
    u16 op;
    u32 n, m;
    if(offset == bytes) return 1;
    if(offset >= bytes || (offset & 1)) return 0;
    op = code_buffer[offset / 2];
    n = (op >> 8) & 15;
    m = (op >> 4) & 15;
    if(op == 0x0009) { /* nop */ }
    else if((op & 0xF000) == 0xE000)
      regs[n] = (u32)(int32_t)(int8_t)op;
    else if((op & 0xF000) == 0x7000)
      regs[n] += (u32)(int32_t)(int8_t)op;
    else if((op & 0xF0FF) == 0x4018) regs[n] <<= 8;
    else if((op & 0xF00F) == 0x600C) regs[n] = regs[m] & 255;
    else if((op & 0xF00F) == 0x6003) regs[n] = regs[m];
    else if((op & 0xF00F) == 0x6007) regs[n] = ~regs[m];
    else if((op & 0xF00F) == 0x300C) regs[n] += regs[m];
    else if((op & 0xF00F) == 0x3008) regs[n] -= regs[m];
    else if((op & 0xF00F) == 0x2009) regs[n] &= regs[m];
    else if((op & 0xF00F) == 0x200A) regs[n] ^= regs[m];
    else if((op & 0xF00F) == 0x200B) regs[n] |= regs[m];
    else if((op & 0xFF00) == 0x8900 || (op & 0xFF00) == 0x8B00)
    {
      if(t == ((op & 0xFF00) == 0x8900))
        next = pc + 4 + 2 * (int32_t)(int8_t)op;
    }
    else if((op & 0xF000) == 0xA000 || (op & 0xF0FF) == 0x402B)
    {
      /* Both bra and jmp execute the next instruction before branching. */
      if(offset + 4 > bytes || code_buffer[offset / 2 + 1] != 0x0009)
        return 0;
      if((op & 0xF000) == 0xA000)
      {
        s32 disp = op & 0x0FFF;
        if(disp & 0x800) disp -= 4096;
        next = pc + 4 + disp * 2;
      }
      else next = regs[n];
    }
    else if((op & 0xF000) == 0xD000)
    {
      u32 literal = ((pc + 4) & ~3u) + (op & 255) * 4 - base;
      if(literal > bytes || bytes - literal < 4) return 0;
      memcpy(&regs[n], (u8 *)code_buffer + literal, 4);
    }
    else return 0;
    pc = next;
  }
  return 0;
}

static void test_executed_emission(void)
{
  u32 regs[16], seed = 0x12345678, iteration, r, shape, t, align;
  static const u32 edges[] = {0x100, 0x7fff, 0x8000, 0xffff,
   0x10000, 0x7fffff, 0x800000, 0xffffff, 0x1000000, 0x80000000,
   0xffffff7f};
  int before = failures;
  for(iteration = 0; iteration < 2048; iteration++)
  {
    u32 value = iteration < 512 ? iteration - 256 :
     iteration < 512 + sizeof(edges) / sizeof(edges[0]) ?
     edges[iteration - 512] : seed;
    seed = seed * 1664525u + 1013904223u;
    for(r = 0; r < 16; r++)
    {
      memset(regs, 0x5A, sizeof(regs));
      reset_buffer();
      SH4_EMIT_LOAD_IMM(r, value);
      if(!simulate_emission(regs, 0) || regs[r] != value)
        failures++;
      {
        u32 other;
        for(other = 0; other < 16; other++)
          if(other != r && regs[other] != 0x5A5A5A5A) failures++;
      }
      if(value == 0x100 && translation_ptr - code_buffer != 2) failures++;
      if(value == 0x10000 && translation_ptr - code_buffer != 3) failures++;
    }
  }
  for(shape = 0; shape < 4; shape++)
    for(t = 0; t < 2; t++)
      for(align = 0; align < 2; align++)
      {
        u8 *branch;
        memset(regs, 0, sizeof(regs));
        reset_buffer();
        if(align) SH4_EMIT_NOP();
        if(shape == 0) SH4_EMIT_COND_SKIP_T(branch);
        if(shape == 1) SH4_EMIT_COND_SKIP_F(branch);
        if(shape == 2) SH4_EMIT_COND_SKIP_T_FAR(branch);
        if(shape == 3) SH4_EMIT_COND_SKIP_F_FAR(branch);
        SH4_EMIT_MOVI(sh4_reg_r4, 1);
        if(shape < 2)
          generate_branch_patch_conditional(branch, translation_ptr);
        else
          /* Exercise the literal veneer even though this target is near. */
          *sh4_long_branch_literal(branch) = (u32)(uintptr_t)translation_ptr;
        if(!simulate_emission(regs, t) || regs[4] != (t != !(shape & 1)))
          failures++;
      }
  printf("executed immediate/conditional sequences: %s\n",
   failures == before ? "ok" : "FAILED");
}
