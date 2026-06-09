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

static void test_stability_contract(void)
{
  char *memory_c = read_text_file("../memory.c");
  char *main_c = read_text_file("../main.c");
  char *common_h = read_text_file("../common.h");
  char *gui_c = read_text_file("../gui.c");
  char *video_c = read_text_file("../video.c");
  char *cpu_c = read_text_file("../cpu_threaded.c");
  char *sh4_c = read_text_file("../dc/sh4_stub.c");
  char *sound_c = read_text_file("../sound.c");
  char *zip_c = read_text_file("../zip.c");

  expect_contains("file read ok macro", common_h, "file_read_ok");
  expect_contains("backup size cap", memory_c, "backup_cap");
  expect_contains("gamepak open path bounds", memory_c,
   "snprintf(open_path, sizeof(open_path)");
  expect_contains("flash bank repair", memory_c, "memory_repair_flash_bank_ptr");
  expect_contains("load state reload loop", memory_c, "reload_attempted");
  expect_contains("paged rom read guard", memory_c, "file_read_ok(gamepak_file_large");
  expect_contains("boot requires rom", main_c, "if(gamepak_filename[0] == 0)");
  expect_contains("copy screen null guard", video_c, "if(copy == NULL)");
  expect_contains("browser alloc failure", gui_c, "browser_alloc_failed");
  expect_contains("external exit bounds", cpu_c,
   "external_block_exit_position >= MAX_EXITS");
  expect_contains("translation redo limit", cpu_c, "translation_redo_attempts");
  expect_contains("null dynarec target guard", sh4_c, "if(target == NULL)");
  expect_contains("sound timer mutex", sound_c, "SDL_LockMutex(sound_mutex)");
  expect_contains("zip filename cap", zip_c, "sizeof(tmp)");

  if(memory_c != NULL)
    free(memory_c);
  if(main_c != NULL)
    free(main_c);
  if(common_h != NULL)
    free(common_h);
  if(gui_c != NULL)
    free(gui_c);
  if(video_c != NULL)
    free(video_c);
  if(cpu_c != NULL)
    free(cpu_c);
  if(sh4_c != NULL)
    free(sh4_c);
  if(sound_c != NULL)
    free(sound_c);
  if(zip_c != NULL)
    free(zip_c);

  printf("stability contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_stability_contract();

  if(failures != 0)
  {
    printf("phase9_stability_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
