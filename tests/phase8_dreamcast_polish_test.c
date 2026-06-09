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

static void test_audio_contract(void)
{
  char *sound_c = read_text_file("../sound.c");
  char *main_h = read_text_file("../main.h");

  expect_contains("direct sound fifo channel", sound_c,
   "direct_sound_channel + channel");
  expect_contains("dreamcast audio buffer", sound_c,
   "defined(_arch_dreamcast)");
  expect_contains("sound reset fifo channel", sound_c,
   "direct_sound_channel + channel");
  expect_contains("audio off null copy", sound_c, "sound_copy_null");
  expect_contains("audio init error decl", main_h, "gpsp_audio_init_error");
  expect_contains("sound init guard", sound_c, "sound_initialized");
  expect_contains("fifo reset indices", sound_c, "ds->fifo_fractional = 0");
  expect_contains("full ring buffer reset", sound_c, "sizeof(sound_buffer)");

  if(sound_c != NULL)
    free(sound_c);
  if(main_h != NULL)
    free(main_h);

  printf("audio contract: ok\n");
}

static void test_video_contract(void)
{
  char *video_c = read_text_file("../video.c");

  expect_contains("copy screen pitch", video_c, "get_screen_pitch()");
  expect_contains("frameskip affine update", video_c, "if(!skip_next_frame)");
  expect_contains("dreamcast vblank wait", video_c,
   "SDL_DC_VerticalWait(SDL_FALSE)");
  expect_contains("obj priority bounds", video_c, "current_count < 128");
  expect_contains("blit memcpy path", video_c, "blit_to_screen_pitch_copy");

  if(video_c != NULL)
    free(video_c);

  printf("video contract: ok\n");
}

static void test_menu_contract(void)
{
  char *gui_c = read_text_file("../gui.c");
  char *input_c = read_text_file("../input.c");
  char *main_h = read_text_file("../main.h");
  char *video_c = read_text_file("../video.c");

  expect_contains("menu dirty redraw", gui_c, "menu_dirty");
  expect_contains("savestate preview deferral", gui_c,
   "savestate_preview_dirty");
  expect_contains("savestate slot sync", gui_c, "menu_sync_savestate_slot");
  expect_contains("dreamcast menu input delay", input_c,
   "#ifndef _arch_dreamcast");
  expect_contains("dreamcast menu exit label", gui_c, "\"Exit gPSPDC\"");
  expect_contains("dreamcast gamepad labels", gui_c, "\"A button     \"");
  expect_contains("dreamcast scaling help", gui_c,
   "displayed on Dreamcast");
  expect_contains("menu saves config on exit", gui_c,
   "save_config_file();");
  expect_contains("rom browser dc help", gui_c,
   "B: cancel   X: parent folder");
  expect_contains("dc gamepad config in gameplay", input_c,
   "dc_process_special_button");
  expect_contains("dc menu button mapping", input_c,
   "BUTTON_ID_MENU");
  expect_contains("dc joy button mapping", input_c, "dc_joy_button_bit");
  expect_contains("rapidfire R support", input_c, "BUTTON_ID_RAPIDFIRE_R");
  expect_contains("video init error decl", main_h, "gpsp_video_init_error");
  expect_contains("video init fatal path", video_c, "gpsp_video_init_error");

  if(gui_c != NULL)
    free(gui_c);
  if(input_c != NULL)
    free(input_c);
  if(main_h != NULL)
    free(main_h);
  if(video_c != NULL)
    free(video_c);

  printf("menu contract: ok\n");
}

static void test_phase8_doc_contract(void)
{
  char *doc = read_text_file("../HIGH_IMPACT_FIXES.md");

  expect_contains("phase 8 doc", doc, "Phase 8");
  expect_contains("phase 8 audio", doc, "sound_reset_fifo");
  expect_contains("phase 8 menu", doc, "Menu performance");

  if(doc != NULL)
    free(doc);

  printf("phase 8 doc contract: ok\n");
}

static void test_build_contract(void)
{
  char *makefile = read_text_file("../dc/Makefile");
  char *build_script = read_text_file("../scripts/dc-build.sh");
  char *root_makefile = read_text_file("../Makefile");

  expect_contains("dc makefile VPATH", makefile, "VPATH += .. .");
  expect_contains("pinned docker image", build_script,
   "gcc-9__v2.0.0");
  expect_contains("safe dc blit default", makefile, "GPSP_DC_BLIT_MEMCPY");
  expect_contains("root makefile delegates x86", root_makefile,
   "$(MAKE) -C x86");

  if(makefile != NULL)
    free(makefile);
  if(build_script != NULL)
    free(build_script);
  if(root_makefile != NULL)
    free(root_makefile);

  printf("build polish contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_audio_contract();
  test_video_contract();
  test_menu_contract();
  test_phase8_doc_contract();
  test_build_contract();

  if(failures != 0)
  {
    printf("phase8_dreamcast_polish_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
