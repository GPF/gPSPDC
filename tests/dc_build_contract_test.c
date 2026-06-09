#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static void expect_path_exists(const char *path)
{
  struct stat st;

  if(stat(path, &st) != 0)
  {
    printf("missing path: %s\n", path);
    failures++;
  }
}

static void test_makefile_sources(void)
{
  static const char *sources[] =
  {
    "../dc/sh4_stub.c",
    "../dc/sh4_helpers.c",
    "../main.c",
    "../cpu_threaded.c",
    "../cpu.c",
    "../memory.c",
    "../video.c",
    "../input.c",
    "../sound.c",
    "../gui.c",
    "../zip.c",
    "../cheats.c",
    NULL
  };
  size_t i;

  expect_path_exists("../dc/Makefile");
  expect_path_exists("../dc/romdisk");

  for(i = 0; sources[i] != NULL; i++)
    expect_path_exists(sources[i]);

  printf("makefile sources contract: ok\n");
}

static void test_dreamcast_ci_contract(void)
{
  char *workflow = read_text_file("../.github/workflows/dreamcast-build.yml");

  expect_contains("dreamcast CI workflow", workflow, "gdC.elf");
  expect_contains("dreamcast CI make", workflow, "-w /src/dc");
  expect_contains("dreamcast CI image", workflow,
   "einsteinx2/dcdev-kos-toolchain:gcc-9");

  if(workflow != NULL)
    free(workflow);

  printf("dreamcast CI contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_makefile_sources();
  test_dreamcast_ci_contract();

  if(failures != 0)
  {
    printf("dc_build_contract_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
