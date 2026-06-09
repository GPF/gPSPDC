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

static void test_version_contract(void)
{
  char *common_h = read_text_file("../common.h");
  char *gui_c = read_text_file("../gui.c");

  expect_contains("version define", common_h, "#define GPSPDC_VERSION");
  expect_contains("version in menu", gui_c, "\"gPSPDC \" GPSPDC_VERSION");

  if(common_h != NULL)
    free(common_h);
  if(gui_c != NULL)
    free(gui_c);

  printf("version contract: ok\n");
}

static void test_menu_load_error_contract(void)
{
  char *gui_c = read_text_file("../gui.c");
  char *main_h = read_text_file("../main.h");

  expect_contains("menu load DC error", gui_c,
   "gpsp_gamepak_load_error((char *)load_filename)");
  expect_contains("gamepak error decl", main_h, "gpsp_gamepak_load_error");

  if(gui_c != NULL)
    free(gui_c);
  if(main_h != NULL)
    free(main_h);

  printf("menu load error contract: ok\n");
}

static void test_smoke_test_doc_contract(void)
{
  char *doc = read_text_file("../HARDWARE_SMOKE_TEST.md");

  expect_contains("smoke test BIOS", doc, "gba_bios.bin");
  expect_contains("smoke test no ROM", doc, "No game loaded yet");
  expect_contains("smoke test menu load", doc, "ROM from the menu");

  if(doc != NULL)
    free(doc);

  printf("smoke test doc contract: ok\n");
}

static void test_ci_contract(void)
{
  char *host_tests = read_text_file("../.github/workflows/tests.yml");
  char *dreamcast_build = read_text_file("../.github/workflows/dreamcast-build.yml");

  expect_contains("host CI workflow", host_tests, "make -C tests test");
  expect_contains("dreamcast CI workflow", dreamcast_build, "gdC.elf");

  if(host_tests != NULL)
    free(host_tests);
  if(dreamcast_build != NULL)
    free(dreamcast_build);

  printf("CI contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_version_contract();
  test_menu_load_error_contract();
  test_smoke_test_doc_contract();
  test_ci_contract();

  if(failures != 0)
  {
    printf("phase6_release_verification_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
