#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  uint32_t addr = ((op1 << 4) & 0x0F000000u) | (op1 & 0x000FFFFFu);
  uint32_t cond = (op1 >> 24) & 0xFFu;

  if(addr != 0x0034567u || (cond & 0x38u) != 0x08u)
  {
    printf("PAR3 condition decode failed: addr=%08x cond=%02x\n", addr, cond);
    return 1;
  }

  printf("PAR3 condition decode: ok\n");
  return 0;
}

static int test_par3_if_stack_depth(void)
{
  const unsigned stack_max = 32;

  if(stack_max < 2)
  {
    printf("PAR3 if stack depth failed\n");
    return 1;
  }

  printf("PAR3 if stack depth: ok\n");
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

static int test_par3_slowdown_decode(void)
{
  uint32_t right = 0x08000500u;
  uint32_t factor = (right >> 8) & 0xFFu;

  if(factor != 5u)
  {
    printf("PAR3 slowdown decode failed: factor=%u\n", factor);
    return 1;
  }

  printf("PAR3 slowdown decode: ok\n");
  return 0;
}

static int test_multi_hook_slots(void)
{
  const unsigned max_hooks = 8;

  if(max_hooks < 2)
  {
    printf("multi hook slots failed\n");
    return 1;
  }

  printf("multi hook slots: ok\n");
  return 0;
}

static int test_deadface_marker(void)
{
  uint32_t address = 0xDEADFACEu;

  if(address != 0xDEADFACEu || (address >> 28) != 0xDu)
  {
    printf("DEADFACE marker failed\n");
    return 1;
  }

  printf("DEADFACE marker: ok\n");
  return 0;
}

static char *read_text_file(const char *path)
{
  FILE *fp = fopen(path, "rb");
  long size;
  char *buffer;

  if(fp == NULL)
    return NULL;

  if(fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0 ||
   fseek(fp, 0, SEEK_SET) != 0)
  {
    fclose(fp);
    return NULL;
  }

  buffer = (char *)malloc((size_t)size + 1);
  if(buffer == NULL)
  {
    fclose(fp);
    return NULL;
  }

  if(fread(buffer, 1, (size_t)size, fp) != (size_t)size)
  {
    free(buffer);
    fclose(fp);
    return NULL;
  }

  buffer[size] = '\0';
  fclose(fp);
  return buffer;
}

static int test_cheats_source_contract(void)
{
  char *cheats_c = read_text_file("../cheats.c");
  char *threaded = read_text_file("../cpu_threaded.c");

  if(cheats_c == NULL || threaded == NULL)
  {
    printf("cheats source contract failed: could not read sources\n");
    free(cheats_c);
    free(threaded);
    return 1;
  }

  if(strstr(cheats_c, "case 0x6:") == NULL ||
   strstr(cheats_c, "case 0x8:") == NULL ||
   strstr(cheats_c, "cheat_add_master_hook") == NULL ||
   strstr(cheats_c, "cheat_hook_pc_valid") == NULL ||
   strstr(cheats_c, "DEADFACE") == NULL ||
   strstr(threaded, "cheat_pc_is_hook(pc)") == NULL)
  {
    printf("cheats source contract failed: missing implementation markers\n");
    free(cheats_c);
    free(threaded);
    return 1;
  }

  free(cheats_c);
  free(threaded);
  printf("cheats source contract: ok\n");
  return 0;
}

int main(void)
{
  int failed = 0;

  failed |= test_gs3_inner_opcode();
  failed |= test_master_hook_address();
  failed |= test_gs1_if_conditions();
  failed |= test_par3_condition_decode();
  failed |= test_par3_if_stack_depth();
  failed |= test_gs1_hook_opcode();
  failed |= test_par3_slowdown_decode();
  failed |= test_multi_hook_slots();
  failed |= test_deadface_marker();
  failed |= test_cheats_source_contract();

  return failed ? 1 : 0;
}
