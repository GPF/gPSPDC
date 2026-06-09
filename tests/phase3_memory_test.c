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

static void expect_count(const char *name, const char *text, const char *needle,
 int expected)
{
  const char *cursor;
  int count = 0;

  if(text == NULL)
    return;

  for(cursor = text; (cursor = strstr(cursor, needle)) != NULL; cursor++)
    count++;

  if(count != expected)
  {
    printf("%s failed: expected %d of `%s`, found %d\n",
     name, expected, needle, count);
    failures++;
  }
}

static void test_memory_contract(void)
{
  char *memory_c = read_text_file("../memory.c");
  char *main_c = read_text_file("../main.c");
  char *cpu_h = read_text_file("../cpu.h");

  expect_contains("32KB page size", memory_c, "GAMEPAK_SWAP_PAGE_SIZE");
  expect_contains("dc buffer tiers", memory_c, "16 * 1024 * 1024");
  expect_contains("dc buffer fallback", memory_c, "4 * 1024 * 1024");
  expect_contains("rom buffer null guard", memory_c, "if(gamepak_rom == NULL)");
  expect_contains("memory map null guard", memory_c,
   "if(gamepak_memory_map == NULL)");
  expect_contains("dc fatal on no rom buffer", main_c, "gpsp_no_memory_error");
  expect_contains("adjacent page prefetch", memory_c,
   "prefetch_adjacent_gamepak_page");
  expect_count("single bios_rom definition", memory_c, "u8 bios_rom[", 1);
  expect_contains("dc smaller rom translation cache", cpu_h,
   "#ifdef _arch_dreamcast");
  expect_contains("dc rom cache size", cpu_h, "1024 * 256 * 4");

  if(memory_c != NULL)
    free(memory_c);
  if(main_c != NULL)
    free(main_c);
  if(cpu_h != NULL)
    free(cpu_h);

  printf("memory contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_memory_contract();

  if(failures != 0)
  {
    printf("phase3_memory_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
