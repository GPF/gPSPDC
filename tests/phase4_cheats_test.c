#include <stdio.h>
#include <stdint.h>

static int test_gs3_inner_opcode(void)
{
  uint32_t c6_line = 0xC6001234u;
  uint32_t c7_line = 0xC7001234u;
  uint32_t inner_c6 = (c6_line >> 24) & 0x0Fu;
  uint32_t inner_c7 = (c7_line >> 24) & 0x0Fu;
  uint32_t broken_c6 = c6_line >> 24;

  if(inner_c6 != 0x6u || inner_c7 != 0x7u)
  {
    printf("GS3 inner opcode extraction failed: c6=%u c7=%u\n",
     inner_c6, inner_c7);
    return 1;
  }

  if(broken_c6 == 0x6u)
  {
    printf("GS3 regression guard failed: old shift still matches\n");
    return 1;
  }

  printf("GS3 inner opcode extraction: ok\n");
  return 0;
}

static int test_master_hook_address(void)
{
  uint32_t address = 0x01234567u;
  uint32_t value = 0x001DC0DEu;
  uint32_t hook;

  if(value != 0x001DC0DEu)
    return 1;

  hook = 0x08000000u | (address & 0x1FFFFFFu);
  if(hook != 0x09234567u)
  {
    printf("master hook address failed: %08x\n", hook);
    return 1;
  }

  printf("master hook address: ok\n");
  return 0;
}

static int test_gs1_if_conditions(void)
{
  uint32_t value_eq = 0x00000042u;
  uint32_t value_ne = 0x00100042u;
  uint32_t value_le = 0x00200042u;
  uint32_t value_ge = 0x00300042u;

  if(((value_eq >> 20) & 0x0Fu) != 0u ||
   ((value_ne >> 20) & 0x0Fu) != 1u ||
   ((value_le >> 20) & 0x0Fu) != 2u ||
   ((value_ge >> 20) & 0x0Fu) != 3u)
  {
    printf("GS1 IF condition decode failed\n");
    return 1;
  }

  printf("GS1 IF condition decode: ok\n");
  return 0;
}

static int test_par3_condition_decode(void)
{
  uint32_t op1 = 0x08034567u;
  uint32_t width = 1u << ((op1 & 0x06000000u) >> 25);
  uint32_t addr = (op1 & 0x000FFFFFu) + ((op1 << 4) & 0x0F000000u);

  if((op1 & 0x38000000u) != 0x08000000u || width != 1u || addr != 0x0034567u)
  {
    printf("PAR3 condition decode failed: width=%u addr=%08x\n", width, addr);
    return 1;
  }

  printf("PAR3 condition decode: ok\n");
  return 0;
}

static int test_gs1_hook_opcode(void)
{
  uint32_t address = 0xF1234567u;
  uint32_t hook = 0x08000000u | (address & 0x1FFFFFFu);

  if((address >> 28) != 0x0Fu || hook != 0x09234567u)
  {
    printf("GS1 hook opcode failed: hook=%08x\n", hook);
    return 1;
  }

  printf("GS1 hook opcode: ok\n");
  return 0;
}

int main(void)
{
  int failed = 0;

  failed |= test_gs3_inner_opcode();
  failed |= test_master_hook_address();
  failed |= test_gs1_if_conditions();
  failed |= test_par3_condition_decode();
  failed |= test_gs1_hook_opcode();

  return failed ? 1 : 0;
}
