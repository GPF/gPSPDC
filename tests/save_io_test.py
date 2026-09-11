"""Compile actual save/config functions with fault-injecting I/O and UI shims.

No SDL/KOS runtime is required. Run from any directory with Python 3 and a C
compiler on PATH: python tests/save_io_test.py [--cc gcc|clang|cl].
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent


def function(source, signature):
    start = source.index(signature)
    body = source.index("{", start)
    depth = 1
    end = body + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end] + "\n"


SHIMS = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
#define MAX_CHEATS 10
#define GPSP_CONFIG_FILENAME "gpsp.cfg"
#define BUTTON_ID_NONE 15
#define BUTTON_ID_MENU 3
#define auto_frameskip 1
static u32 current_frameskip_type, frameskip_value, random_skip, clock_speed;
static u32 screen_scale, screen_filter, global_enable_audio;
static u32 audio_buffer_size_number, update_backup_flag, global_enable_analog;
static u32 analog_sensitivity_level, gamepad_config_map[16];
static struct { u32 cheat_active; } cheats[MAX_CHEATS];
static char main_path[512] = ".";
static u8 gamepak_filename[512] = "game.gba";
static u8 backup_filename[512] = "game.sav";
static int opens, closes, writes, notices, fail_open, fail_read, fail_write;
static int fail_close, live_handles;
static u32 size_on_disk, values[23];
static int io_open(void) {
  opens++;
  if(fail_open) return 0;
  live_handles++;
  return opens;
}
static int io_read(void *buffer, size_t size) {
  if(fail_read) return 0;
  memcpy(buffer, values, size);
  return 1;
}
static int io_close(void) { closes++; live_handles--; return fail_close ? -1 : 0; }
static int io_write(void) { writes++; return !fail_write; }
#define file_open(tag, name, mode) int tag = io_open()
#define file_check_valid(tag) (tag)
#define file_length(name, tag) size_on_disk
#define file_read_ok(tag, buffer, size) io_read(buffer, size)
#define file_close(tag) io_close()
#define file_write_ok(tag, buffer, size) io_write()
static void change_ext(const void *in, void *out, const void *ext) {
  strcpy(out, in); strcat(out, ext);
}
static void gpsp_save_error(const char *kind, const char *name) { notices++; }
enum { BACKUP_NONE, BACKUP_SRAM, BACKUP_FLASH, BACKUP_EEPROM };
enum { SRAM_SIZE_32KB, FLASH_SIZE_64KB, EEPROM_512_BYTE };
static u32 backup_type, sram_size, flash_size, eeprom_size, backup_update;
static u8 gamepak_backup[131072];
static const u32 write_backup_delay = 10;
static u32 backup_retry_frames, backup_failure_reported;
static int sound_initialized = 1, sound_mutex, locked, paused;
static void SDL_LockMutex(int x) { locked++; }
static void SDL_UnlockMutex(int x) { locked--; }
static void SDL_PauseAudio(int x) { paused = x; }
static u8 savestate_write_buffer[506947], *write_mem_ptr;
#define file_write_mem(tag, buffer, size) do { memcpy(write_mem_ptr, buffer, size); write_mem_ptr += size; } while(0)
#define file_write_mem_variable(tag, value) file_write_mem(tag, &(value), sizeof(value))
#define savestate_block(type) ((void)0)
static void reset_io(void) {
  assert(live_handles == 0);
  opens = closes = writes = notices = 0;
  fail_open = fail_read = fail_write = fail_close = 0;
}
'''

CHECKS = r'''
int main(void) {
  int i, attempt;
  static u16 capture[240 * 160];
  for(i = 0; i < 1000; i++) {
    size_on_disk = 1;
    assert(load_config_file() == -1);
    assert(load_game_config_file() == -1);
    assert(live_handles == 0);
  }
  assert(opens == closes);
  reset_io();
  screen_scale = 2;
  size_on_disk = 92; fail_read = 1;
  assert(load_config_file() == -1 && screen_scale == 2);
  size_on_disk = 56;
  assert(load_game_config_file() == -1 && frameskip_value == 4);
  reset_io();
  size_on_disk = 92; fail_close = 1;
  assert(load_config_file() == -1 && screen_scale == 2);
  reset_io();
  size_on_disk = 92; values[0] = 1;
  assert(load_config_file() == 0 && screen_scale == 1);
  assert(gamepad_config_map[0] == BUTTON_ID_MENU);
  size_on_disk = 56; values[1] = 999; values[3] = 999;
  assert(load_game_config_file() == 0 && frameskip_value == 99 && clock_speed == 333);
  assert(live_handles == 0);
  reset_io();
  fail_open = 1;
  assert(load_config_file() == -1 && load_game_config_file() == -1);
  assert(closes == 0 && live_handles == 0);
  for(attempt = 0; attempt < 4; attempt++) {
    reset_io();
    fail_open = attempt == 1; fail_write = attempt == 2; fail_close = attempt == 3;
    assert(save_config_file() == (attempt ? -1 : 0));
    assert(live_handles == 0);
    backup_type = BACKUP_SRAM;
    assert(save_backup((char *)backup_filename) == (attempt ? 0 : 1));
    assert(save_state("test.svs", capture) == (attempt ? -1 : 0));
    assert(locked == 0 && paused == 0 && live_handles == 0);
    if(attempt) assert(notices > 0);
  }
  reset_io();
  gamepak_filename[0] = 0;
  assert(save_game_config_file() == 0 && opens == 0);
  reset_io();
  backup_type = BACKUP_SRAM; backup_update = 0; fail_write = 1;
  update_backup();
  assert(writes == 1 && notices == 1 && backup_update == 0);
  for(i = 0; i < 300; i++) update_backup();
  assert(writes == 1 && notices == 1);
  update_backup();
  assert(writes == 2 && notices == 1 && backup_update == 0);
  /* New game writes must not bypass the retry backoff. */
  for(i = 0; i < 50; i++) {
    backup_update = write_backup_delay;
    update_backup();
  }
  assert(writes == 2 && notices == 1);
  fail_write = 0;
  for(i = 0; i < 301; i++) update_backup();
  assert(writes == 3 && backup_update == write_backup_delay + 1);
  for(i = 0; i < 600; i++) update_backup();
  assert(writes == 3);
  fail_open = 1;
  update_backup_force();
  assert(backup_update == 0);
  fail_open = 0;
  update_backup_force();
  assert(backup_update == write_backup_delay + 1 && backup_retry_frames == 0);
  reset_io();
  backup_type = BACKUP_NONE; backup_update = 0;
  update_backup(); update_backup_force();
  assert(opens == 0 && notices == 0);
  puts("save/config fault-injection tests passed");
  return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", default="cc")
    args = parser.parse_args()
    gui = (ROOT / "gui.c").read_text()
    memory = (ROOT / "memory.c").read_text()
    code = SHIMS
    for name in ("load_game_config_file", "load_config_file",
                 "save_game_config_file", "save_config_file"):
        code += function(gui, "s32 " + name + "()")
    code += function(memory, "u32 save_backup(")
    for name in ("update_backup", "update_backup_force"):
        code += function(memory, "void " + name + "()")
    code += function(memory, "s32 save_state(")
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory) / "save_io.c"
        binary = Path(directory) / "save_io.exe"
        source.write_text(code + CHECKS)
        if Path(args.cc).stem.lower() == "cl":
            cmd = [args.cc, "/nologo", "/std:c11", "/D_CRT_SECURE_NO_WARNINGS",
                   str(source), "/Fe:" + str(binary)]
        else:
            cmd = [args.cc, "-std=c99", str(source), "-o", str(binary)]
        subprocess.run(cmd, cwd=directory, check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
