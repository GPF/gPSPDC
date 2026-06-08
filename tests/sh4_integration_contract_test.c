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

  if(fseek(fp, 0, SEEK_END) != 0)
  {
    printf("failed to seek %s\n", path);
    fclose(fp);
    failures++;
    return NULL;
  }

  size = ftell(fp);
  if(size < 0 || fseek(fp, 0, SEEK_SET) != 0)
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

static int count_occurrences(const char *haystack, const char *needle)
{
  int count = 0;
  const char *cursor = haystack;
  size_t needle_len = strlen(needle);

  while((cursor = strstr(cursor, needle)) != NULL)
  {
    count++;
    cursor += needle_len;
  }

  return count;
}

static void expect_contains(const char *name, const char *text,
 const char *needle)
{
  if(strstr(text, needle) == NULL)
  {
    printf("%s failed: missing `%s`\n", name, needle);
    failures++;
  }
}

static void expect_count(const char *name, const char *text, const char *needle,
 int expected)
{
  int count = count_occurrences(text, needle);

  if(count != expected)
  {
    printf("%s failed: `%s` count=%d expected=%d\n", name, needle, count,
     expected);
    failures++;
  }
}

static void test_sh4_stub_async_exit_contract(void)
{
  char *text = read_text_file("../dc/sh4_stub.c");
  int failures_before = failures;

  if(text == NULL)
    return;

  expect_contains("cycle reload helper", text,
   "static inline void sh4_reload_cycles(u32 cycles)");
  expect_contains("cycle reload register", text, "mov.l %0, r13");
  expect_count("cycle reload call sites", text, "sh4_reload_cycles(cycles);",
   4);
  expect_count("update_gba cycle capture", text, "cycles = update_gba();",
   4);
  expect_count("old store update_gba result capture", text,
   "result = update_gba();", 0);
  expect_count("shared pc lookup helper", text, "static void sh4_lookup_pc(void)",
   1);

  free(text);
  if(failures == failures_before)
    printf("SH-4 async exit contract: ok\n");
}

static void test_sh4_helpers_irq_contract(void)
{
  char *text = read_text_file("../dc/sh4_helpers.c");
  int failures_before = failures;

  if(text == NULL)
    return;

  expect_contains("irq helper", text, "static u32 sh4_take_pending_irq(u32 return_pc)");
  expect_contains("store cpsr return type", text,
   "u32 function_cc execute_store_cpsr(u32 new_cpsr, u32 store_mask, u32 pc)");
  expect_contains("spsr restore irq redirect", text,
   "irq_pc = sh4_take_pending_irq(address);");
  expect_count("old broken check_for_interrupts macro", text,
   "#define check_for_interrupts()", 0);

  free(text);
  if(failures == failures_before)
    printf("SH-4 helpers irq contract: ok\n");
}

static void test_sh4_psr_store_contract(void)
{
  char *text = read_text_file("../dc/sh4_instr.inc");
  int failures_before = failures;

  if(text == NULL)
    return;

  expect_contains("cpsr store finish", text, "#define arm_psr_store_finish(cpsr)");
  expect_contains("cpsr irq branch postamble", text,
   "#define arm_psr_store_cpsr_post()");
  expect_contains("cpsr store pc arg", text, "generate_load_pc(a2, pc);");
  expect_contains("cpsr irq indirect branch", text,
   "generate_indirect_branch_arm();");

  free(text);
  if(failures == failures_before)
    printf("SH-4 psr store irq contract: ok\n");
}

static void test_skyemu_cheat_contract(void)
{
  char *text = read_text_file("../cheats.c");
  int failures_before = failures;

  if(text == NULL)
    return;

  expect_contains("skyemu attribution", text, "adapted from SkyEmu");
  expect_contains("par3 if stack", text, "if_stack[PAR3_IF_STACK_MAX]");
  expect_contains("par3 ar if helper", text,
   "static u32 par3_handle_ar_if(u32 left, u32 right)");
  expect_contains("par3 else opcode", text, "case 0x60:");

  free(text);
  if(failures == failures_before)
    printf("SkyEmu cheat contract: ok\n");
}

static void test_dynarec_cheat_hook_contract(void)
{
  char *threaded = read_text_file("../cpu_threaded.c");
  char *sh4_stub = read_text_file("../dc/sh4_stub.c");
  char *sh4_emit = read_text_file("../dc/sh4_emit.h");
  int failures_before = failures;

  if(threaded != NULL)
  {
    expect_contains("cheat hook pc check", threaded,
     "if(cheat_pc_is_hook(pc))");
    expect_contains("cheat hook emission", threaded,
     "type##_process_cheats();");
    free(threaded);
  }

  if(sh4_stub != NULL)
  {
    expect_contains("sh4 cheat hook helper", sh4_stub, "void sh4_cheat_hook(void)");
    expect_contains("sh4 cheat hook call", sh4_stub, "process_cheats();");
    free(sh4_stub);
  }

  if(sh4_emit != NULL)
  {
    expect_contains("arm cheat process macro", sh4_emit,
     "#define arm_process_cheats()");
    free(sh4_emit);
  }

  if(failures == failures_before)
    printf("dynarec cheat hook contract: ok\n");
}

static void test_translation_cache_invalidation_contract(void)
{
  char *text = read_text_file("../cpu_threaded.c");
  int failures_before = failures;

  if(text == NULL)
    return;

  expect_contains("RAM/BIOS cache invalidation hook", text,
   "translate_invalidate_dcache_region(mem_type##_translation_cache,");
  expect_contains("ROM cache invalidation hook", text,
   "translate_invalidate_dcache_region(rom_translation_cache,");
  expect_count("old zero-arg invalidation hook", text,
   "translate_invalidate_dcache();", 0);

  free(text);
  if(failures == failures_before)
    printf("translation cache invalidation contract: ok\n");
}

int main(void)
{
  test_sh4_stub_async_exit_contract();
  test_sh4_helpers_irq_contract();
  test_sh4_psr_store_contract();
  test_skyemu_cheat_contract();
  test_dynarec_cheat_hook_contract();
  test_translation_cache_invalidation_contract();

  return failures == 0 ? 0 : 1;
}
