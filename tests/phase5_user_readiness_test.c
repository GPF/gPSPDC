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

static void test_dc_fatal_error_contract(void)
{
  char *main_c = read_text_file("../main.c");

  expect_contains("DC fatal error helper", main_c, "gpsp_fatal_error_screen");
  expect_contains("DC missing BIOS screen", main_c, "gpsp_missing_bios_error");
  expect_contains("DC BIOS path hint", main_c, "/cd/gba_bios.bin");
  expect_contains("DC BIOS MD5 hint", main_c, "a860e8c0b6d573d191e4ec7db1b1e4f6");
  expect_contains("DC ROM buffer error", main_c, "gpsp_no_memory_error");
  expect_contains("DC gamepak load error", main_c, "gpsp_gamepak_load_error");
  expect_contains("DC argv load fix", main_c, "gpsp_gamepak_load_error(argv[1])");
  expect_contains("DC dynarec fatal error", main_c, "gpsp_dynarec_fatal_error");

  if(main_c != NULL)
    free(main_c);

  printf("DC fatal error contract: ok\n");
}

static void test_rom_zip_safety_contract(void)
{
  char *memory_c = read_text_file("../memory.c");
  char *zip_c = read_text_file("../zip.c");
  char *cpu_threaded_c = read_text_file("../cpu_threaded.c");
  char *main_h = read_text_file("../main.h");

  expect_contains("extensionless ROM guard", memory_c,
   "if(dot_position && !strcmp(dot_position, \".zip\"))");
  expect_contains("savestate ROM load error", memory_c,
   "gpsp_gamepak_load_error((char *)gamepak_filename)");
  expect_contains("zip extension guard", zip_c, "if(dot == NULL || dot[1] == '\\0')");
  expect_contains("zip malloc guard", zip_c, "if(cbuffer == NULL)");
  expect_contains("zip inflate size check", zip_c,
   "if(stream.total_out != expected_size)");
  expect_contains("dynarec fatal decl", main_h, "gpsp_dynarec_fatal_error");
  expect_contains("dynarec fatal call", cpu_threaded_c,
   "gpsp_dynarec_fatal_error(buffer)");

  if(memory_c != NULL)
    free(memory_c);
  if(zip_c != NULL)
    free(zip_c);
  if(cpu_threaded_c != NULL)
    free(cpu_threaded_c);
  if(main_h != NULL)
    free(main_h);

  printf("ROM/ZIP safety contract: ok\n");
}

static void test_readme_savestate_contract(void)
{
  char *readme = read_text_file("../README.md");

  expect_contains("README savestate ext", readme, ".0.svs");
  expect_contains("README savestate ext range", readme, ".9.svs");

  if(readme != NULL)
    free(readme);

  printf("README savestate contract: ok\n");
}

static void test_input_debug_gating_contract(void)
{
  char *input_c = read_text_file("../input.c");

  expect_contains("F2 palette dump gated", input_c, "#ifdef GPSP_DEBUG");
  expect_contains("F3 translation dump gated", input_c,
   "dump_translation_cache();");
  expect_contains("F2 uses debug printf", input_c,
   "gpsp_debug_printf(\"writing palette RAM");

  if(input_c != NULL)
    free(input_c);

  printf("input debug gating contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_dc_fatal_error_contract();
  test_rom_zip_safety_contract();
  test_readme_savestate_contract();
  test_input_debug_gating_contract();

  if(failures != 0)
  {
    printf("phase5_user_readiness_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
