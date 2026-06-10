#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static char *read_text_file(const char *path)
{
  FILE *fp = fopen(path, "rb");
  long size;
  char *buffer;

  if(fp == NULL)
  {
    printf("failed to open %s\n", path);
    failures++;
    return NULL;
  }

  if(fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0 ||
   fseek(fp, 0, SEEK_SET) != 0)
  {
    printf("failed to size %s\n", path);
    fclose(fp);
    failures++;
    return NULL;
  }

  buffer = (char *)malloc((size_t)size + 1);
  if(buffer == NULL)
  {
    printf("failed to allocate %s\n", path);
    fclose(fp);
    failures++;
    return NULL;
  }

  if(fread(buffer, 1, (size_t)size, fp) != (size_t)size)
  {
    printf("failed to read %s\n", path);
    free(buffer);
    fclose(fp);
    failures++;
    return NULL;
  }

  buffer[size] = '\0';
  fclose(fp);
  return buffer;
}

static void expect_contains(const char *name, const char *text,
 const char *needle)
{
  if(text == NULL)
    return;

  if(strstr(text, needle) == NULL)
  {
    printf("%s failed: missing `%s`\n", name, needle);
    failures++;
  }
}

static void test_interpreter_ldm_stm_contract(void)
{
  char *cpu_c = read_text_file("../cpu.c");

  expect_contains("interpreter writeback after loop", cpu_c,
   "arm_block_memory_apply_writeback(access_type, writeback_type)");
  expect_contains("interpreter load writeback skip", cpu_c,
   "if(!((reg_list >> rn) & 0x01))");
  expect_contains("interpreter user bank", cpu_c,
   "arm_block_memory_user_bank(i, s_bit)");

  if(cpu_c != NULL)
    free(cpu_c);

  printf("interpreter LDM/STM contract: ok\n");
}

static void test_dynarec_ldm_stm_contract(void)
{
  char *sh4_helpers = read_text_file("../dc/sh4_helpers.c");
  char *sh4_instr = read_text_file("../dc/sh4_instr.inc");

  expect_contains("dynarec block memory helper", sh4_helpers,
   "execute_arm_block_memory");
  expect_contains("dynarec user bank helper", sh4_helpers,
   "block_memory_user_bank");
  expect_contains("dynarec load writeback skip", sh4_helpers,
   "if(!(load && ((reg_list >> rn) & 0x01)))");
  expect_contains("dynarec block memory call", sh4_instr,
   "generate_function_call(execute_arm_block_memory)");
  expect_contains("dynarec ldm pc branch", sh4_instr,
   "arm_block_memory_branch_pc_load");
  expect_contains("dynarec ldm pc load from reg", sh4_instr,
   "generate_load_reg(a0, REG_PC)");
  expect_contains("dynarec ldm pc dual branch", sh4_instr,
   "generate_indirect_branch_dual()");

  if(sh4_helpers != NULL)
    free(sh4_helpers);
  if(sh4_instr != NULL)
    free(sh4_instr);

  printf("dynarec LDM/STM contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_interpreter_ldm_stm_contract();
  test_dynarec_ldm_stm_contract();

  if(failures != 0)
  {
    printf("phase2_ldm_stm_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
