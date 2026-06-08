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

  if(text == NULL)
    return;

  expect_contains("cycle reload helper", text,
   "static inline void sh4_reload_cycles(u32 cycles)");
  expect_contains("cycle reload register", text, "mov.l %0, r13");
  expect_count("cycle reload call sites", text, "sh4_reload_cycles(cycles);",
   4);
  expect_count("store update_gba cycle capture", text, "cycles = update_gba();",
   3);
  expect_count("old store update_gba result capture", text,
   "result = update_gba();", 0);
  expect_count("shared pc lookup helper", text, "static void sh4_lookup_pc(void)",
   1);

  free(text);
  printf("SH-4 async exit contract: ok\n");
}

static void test_translation_cache_invalidation_contract(void)
{
  char *text = read_text_file("../cpu_threaded.c");

  if(text == NULL)
    return;

  expect_contains("RAM/BIOS cache invalidation hook", text,
   "translate_invalidate_dcache_region(mem_type##_translation_cache,");
  expect_contains("ROM cache invalidation hook", text,
   "translate_invalidate_dcache_region(rom_translation_cache,");
  expect_count("old zero-arg invalidation hook", text,
   "translate_invalidate_dcache();", 0);

  free(text);
  printf("translation cache invalidation contract: ok\n");
}

int main(void)
{
  test_sh4_stub_async_exit_contract();
  test_translation_cache_invalidation_contract();

  return failures == 0 ? 0 : 1;
}
