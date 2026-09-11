/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"
#include "sound.h"
#include "cpu.h"
#include "video.h"
#include "memory.h"
#include "input.h"
#ifdef PSP_BUILD

//PSP_MODULE_INFO("gpSP", 0x1000, 0, 6);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void vblank_interrupt_handler(u32 sub, u32 *parg);

#endif

timer_type timer[4];

//debug_state current_debug_state = COUNTDOWN_BREAKPOINT;
//debug_state current_debug_state = PC_BREAKPOINT;
u32 breakpoint_value = 0x7c5000;
debug_state current_debug_state = RUN;
//u32 breakpoint_value = 0;

frameskip_type current_frameskip_type = auto_frameskip;
u32 frameskip_value = 4;
u32 random_skip = 0;
u32 global_cycles_per_instruction = 3;

u32 skip_next_frame = 0;

u32 frameskip_counter = 0;

u32 cpu_ticks = 0;
u32 frame_ticks = 0;

u32 execute_cycles = 960;
s32 video_count = 960;
u32 ticks;

u32 arm_frame = 0;
u32 thumb_frame = 0;
u32 last_frame = 0;

u32 cycle_memory_access = 0;
u32 cycle_pc_relative_access = 0;
u32 cycle_sp_relative_access = 0;
u32 cycle_block_memory_access = 0;
u32 cycle_block_memory_sp_access = 0;
u32 cycle_block_memory_words = 0;
u32 cycle_dma16_words = 0;
u32 cycle_dma32_words = 0;
u32 flush_ram_count = 0;
u32 gbc_update_count = 0;
u32 oam_update_count = 0;

u32 synchronize_flag = 1;

u32 update_backup_flag = 1;
u32 clock_speed = 333;
char main_path[512];

#define check_count(count_var)                                                \
  if(count_var < execute_cycles)                                              \
    execute_cycles = count_var;                                               \

#define check_timer(timer_number)                                             \
  if(timer[timer_number].status == TIMER_PRESCALE)                            \
    check_count(timer[timer_number].count);                                   \

#define update_timer(timer_number)                                            \
  if(timer[timer_number].status != TIMER_INACTIVE)                            \
  {                                                                           \
    if(timer[timer_number].status != TIMER_CASCADE)                           \
    {                                                                         \
      timer[timer_number].count -= execute_cycles;                            \
      io_registers[REG_TM##timer_number##D] =                                 \
       -(timer[timer_number].count >> timer[timer_number].prescale);          \
    }                                                                         \
                                                                              \
    if(timer[timer_number].count <= 0)                                        \
    {                                                                         \
      if(timer[timer_number].irq == TIMER_TRIGGER_IRQ)                        \
        irq_raised |= IRQ_TIMER##timer_number;                                \
                                                                              \
      if((timer_number != 3) &&                                               \
       (timer[timer_number + 1].status == TIMER_CASCADE))                     \
      {                                                                       \
        timer[timer_number + 1].count--;                                      \
        io_registers[REG_TM0D + (timer_number + 1) * 2] =                     \
         -(timer[timer_number + 1].count);                                    \
      }                                                                       \
                                                                              \
      if(timer_number < 2)                                                    \
      {                                                                       \
        if(timer[timer_number].direct_sound_channels & 0x01)                  \
          sound_timer(timer[timer_number].frequency_step, 0);                 \
                                                                              \
        if(timer[timer_number].direct_sound_channels & 0x02)                  \
          sound_timer(timer[timer_number].frequency_step, 1);                 \
      }                                                                       \
                                                                              \
      timer[timer_number].count +=                                            \
       (timer[timer_number].reload << timer[timer_number].prescale);          \
    }                                                                         \
  }                                                                           


u8 *file_ext[] = { ".gba", ".bin", ".zip", NULL };

void init_main()
{
  u32 i;

  skip_next_frame = 0;

  for(i = 0; i < 4; i++)
  {
    dma[i].start_type = DMA_INACTIVE;
    dma[i].direct_sound_channel = DMA_NO_DIRECT_SOUND;
    timer[i].status = TIMER_INACTIVE;
    timer[i].reload = 0x10000;
    timer[i].stop_cpu_ticks = 0;
  }

  timer[0].direct_sound_channels = TIMER_DS_CHANNEL_BOTH;
  timer[1].direct_sound_channels = TIMER_DS_CHANNEL_NONE;

  cpu_ticks = 0;
  frame_ticks = 0;

  execute_cycles = 960;
  video_count = 960;
  flush_translation_cache_rom();
  flush_translation_cache_ram();
  flush_translation_cache_bios();
}

#if defined(_arch_dreamcast)
#ifdef GPSP_DC_BOOT_TRACE
#include <dc/video.h>
#include <dc/biosfont.h>

static u32 gpsp_boot_trace_y = 24;

static void gpsp_boot_trace(const char *message)
{
  if(gpsp_boot_trace_y == 24)
  {
    vid_init(DM_640x480, PM_RGB565);
    memset(vram_s, 0, 640 * 480 * 2);
  }

  bfont_draw_str(vram_s + (gpsp_boot_trace_y * 640) + 24, 640, 0,
   (char *)message);
  gpsp_boot_trace_y += 28;
}
#else
#define gpsp_boot_trace(message) ((void)0)
#endif

#ifdef GPSP_DC_RUNTIME_TRACE
#define GPSP_DC_TRACE_EVERY(counter, fmt, ...)                                \
  do                                                                          \
  {                                                                           \
    (counter)++;                                                              \
    if(((counter) & 63) == 1)                                                 \
      printf("[gbaDC trace] " fmt "\n", ##__VA_ARGS__);                     \
  } while(0)

static u32 gpsp_dc_debug_vram_checksum(void)
{
  u32 h = 2166136261u;
  u32 i;
  const u16 *vram16 = (const u16 *)vram;

  for(i = 0; i < (96 * 1024) / 2; i += 97)
  {
    h ^= vram16[i];
    h *= 16777619u;
  }

  return h;
}

static void gpsp_dc_trace_vram_dirty(void)
{
  static u32 last_vram_hash;
  u32 now = gpsp_dc_debug_vram_checksum();

  if(now != last_vram_hash)
  {
    printf("[gbaDC trace] VRAM changed %08x -> %08x\n",
     last_vram_hash, now);
    last_vram_hash = now;
  }
}

static void gpsp_dc_trace_loaded_rom(const char *stage)
{
  char title[13];
  u32 i;

  memset(title, 0, sizeof(title));
  if(gamepak_rom != NULL && gamepak_size >= 0xAC)
  {
    memcpy(title, gamepak_rom + 0xA0, 12);
    for(i = 0; i < 12; i++)
    {
      if(title[i] < 0x20 || title[i] > 0x7E)
        title[i] = '.';
    }
  }

  printf("[gbaDC trace] %s ROM title: %.12s size=%u\n", stage, title,
   gamepak_size);
  printf("[gbaDC trace] %s reset PC=%08x CPSR=%08x mode=%u halt=%u\n",
   stage, reg[REG_PC], reg[REG_CPSR], reg[CPU_MODE], reg[CPU_HALT_STATE]);
}
#else
#define GPSP_DC_TRACE_EVERY(counter, fmt, ...) ((void)(counter))
#define gpsp_dc_trace_vram_dirty() ((void)0)
#define gpsp_dc_trace_loaded_rom(stage) ((void)0)
#endif

static void gpsp_fatal_error_screen(const char **lines, u32 line_count)
{
  u32 i;
  gui_action_type gui_action;

  for(i = 0; i < line_count; i++)
    printf("%s\n", lines[i]);

  init_video();
  init_input();
  video_resolution_large();
  clear_screen(0x0000);
  for(i = 0; i < line_count; i++)
    print_string(lines[i], 0xFFFF, 0x0000, 10, 10 + (i * 10));
  flip_screen();

  gui_action = CURSOR_NONE;
  while(gui_action == CURSOR_NONE)
  {
    gui_action = get_gui_input();
    delay_us(15000);
  }

  SDL_Quit();
  exit(1);
}

static void gpsp_missing_bios_error(void)
{
  static const char *lines[] =
  {
    "gPSPDC requires a GBA BIOS image.",
    "Place gba_bios.bin at /cd/gba_bios.bin",
    "Size: 16384 bytes",
    "MD5: a860e8c0b6d573d191e4ec7db1b1e4f6",
    "Press Start to exit."
  };

  gpsp_fatal_error_screen(lines, 5);
}

static void gpsp_no_memory_error(void)
{
  static const char *lines[] =
  {
    "gPSPDC could not allocate ROM buffer.",
    "Not enough system RAM is available.",
    "Press Start to exit."
  };

  gpsp_fatal_error_screen(lines, 3);
}

void gpsp_gamepak_load_error(const char *filename)
{
  static const char *prefix = "Could not load game ROM:";
  static const char *suffix = "Press Start to exit.";
  char detail[512];
  const char *lines[3];

  snprintf(detail, sizeof(detail), "%s",
   filename ? filename : "(unknown file)");
  lines[0] = prefix;
  lines[1] = detail;
  lines[2] = suffix;
  gpsp_fatal_error_screen(lines, 3);
}

void gpsp_video_init_error(const char *sdl_error)
{
  printf("gPSPDC could not initialize video.\n");

  if(sdl_error && sdl_error[0])
    printf("%s\n", sdl_error);

  printf("Press Start to exit.\n");
  exit(1);
}

void gpsp_audio_init_error(const char *sdl_error)
{
  static const char *prefix = "gPSPDC could not initialize audio.";
  static const char *suffix = "Press Start to exit.";
  char detail[512];
  const char *lines[3];

  if(sdl_error && sdl_error[0])
    snprintf(detail, sizeof(detail), "%s", sdl_error);
  else
    snprintf(detail, sizeof(detail), "Unknown SDL audio error.");

  lines[0] = prefix;
  lines[1] = detail;
  lines[2] = suffix;
  gpsp_fatal_error_screen(lines, 3);
}

void gpsp_dynarec_fatal_error(const char *detail)
{
  static const char *prefix = "Dynarec translation failed:";
  static const char *suffix = "Press Start to exit.";
  const char *lines[3];

  lines[0] = prefix;
  lines[1] = detail;
  lines[2] = suffix;
  gpsp_fatal_error_screen(lines, 3);
}

static s32 gpsp_load_autoload_filename(u8 *load_filename,
 u32 load_filename_size)
{
  u32 i;
  file_open(autoload_file, "/cd/gbaDC/autoload.txt", read);

  if(!file_check_valid(autoload_file))
    return -1;

  if(fgets((char *)load_filename, load_filename_size, autoload_file) == NULL)
  {
    file_close(autoload_file);
    return -1;
  }

  file_close(autoload_file);

  for(i = 0; i < load_filename_size && load_filename[i] != 0; i++)
  {
    if(load_filename[i] == '\r' || load_filename[i] == '\n')
    {
      load_filename[i] = 0;
      break;
    }
  }

  if(load_filename[0] == 0)
    return -1;

  return 0;
}
#endif

int main(int argc, char *argv[])
{
  u8 load_filename[512];
#ifdef _arch_dreamcast
  gpsp_boot_trace("gPSPDC boot: entering main");
  fs_chdir("/cd/gbaDC/");
  gpsp_boot_trace("gPSPDC boot: fs_chdir /cd/gbaDC complete");
#endif

#ifdef PSP_BUILD
  sceKernelRegisterSubIntrHandler(PSP_VBLANK_INT, 0,
   vblank_interrupt_handler, NULL);
  sceKernelEnableSubIntr(PSP_VBLANK_INT, 0);
#else
  //freopen("CON", "wb", stdout);
#endif
  gpsp_debug_printf("init_gamepak_buffer...\n");
  gpsp_boot_trace("gPSPDC boot: init_gamepak_buffer");
  init_gamepak_buffer();
#ifdef _arch_dreamcast
  if(gamepak_rom == NULL)
    gpsp_no_memory_error();
#endif

  // Copy the directory path of the executable into main_path
#ifndef _arch_dreamcast
  getcwd(main_path, sizeof(main_path));
#else
  getcwd(main_path,512);
#endif
  gpsp_debug_printf("load_config_file...\n");
  gpsp_boot_trace("gPSPDC boot: load_config_file");
  load_config_file();

  gamepak_filename[0] = 0;
  gpsp_debug_printf("load_bios...\n");
  gpsp_boot_trace("gPSPDC boot: load_bios");
  if(load_bios("/cd/gba_bios.bin") == -1)
  {
#ifdef PSP_BUILD
    gui_action_type gui_action = CURSOR_NONE;

    printf("Sorry, but gpSP requires a Gameboy Advance BIOS image to run\n");
    printf("correctly. Make sure to get an authentic one (search the web,\n");
    printf("beg other people if you want, but don't hold me accountable\n");
    printf("if you get hated or banned for it), it'll be exactly 16384\n");
    printf("bytes large and should have the following md5sum value:\n\n");
    printf("a860e8c0b6d573d191e4ec7db1b1e4f6\n\n");
    printf("Other BIOS files might work either partially completely, I\n");
    printf("really don't know.\n\n");
    printf("When you do get it name it gba_bios.bin and put it in the\n");
    printf("same directory as this EBOOT.\n\n");
    printf("Good luck. Press any button to exit.\n");

    while(gui_action == CURSOR_NONE)
    {
      gui_action = get_gui_input();
      delay_us(15000);
    }

    quit();
#elif defined(_arch_dreamcast)
    gpsp_missing_bios_error();
#else
    printf("Sorry, but gpSP requires a Gameboy Advance BIOS image to run\n");
    exit(1);
#endif
  }

#ifdef PSP_BUILD
  delay_us(2500000);
#endif
  gpsp_debug_printf("Initialize...\ninit_main\n");
  gpsp_boot_trace("gPSPDC boot: init_main");
  init_main();
  gpsp_debug_printf("init_sound\n");
  gpsp_boot_trace("gPSPDC boot: init_sound");
  init_sound();
  gpsp_debug_printf("init_video\n");
  gpsp_boot_trace("gPSPDC boot: init_video");
  init_video();
  gpsp_debug_printf("init_input\n");
  gpsp_boot_trace("gPSPDC boot: init_input");
  init_input();
  gpsp_debug_printf("video_resolution_large\n");
  gpsp_boot_trace("gPSPDC boot: video_resolution_large");
  video_resolution_large();
#if defined(_arch_dreamcast) && defined(GPSP_DC_RUNTIME_TRACE)
  gpsp_dc_debug_video_test_pattern();
#endif
  gpsp_debug_printf("Loading files...\n");
  gpsp_boot_trace("gPSPDC boot: load_file/menu");
  if(argc > 1)
  {
    if(load_gamepak(argv[1]) == -1)
    {
#ifdef _arch_dreamcast
      gpsp_gamepak_load_error(argv[1]);
#else
      printf("Failed to load gamepak %s, exiting.\n", argv[1]);
      exit(-1);
#endif
    }

    set_gba_resolution(screen_scale);
#ifndef _arch_dreamcast
    video_resolution_small();
#endif

    init_cpu();
    init_memory();
    gpsp_dc_trace_loaded_rom("argv load");
  }
  else
  {

#ifdef _arch_dreamcast
    if(gpsp_load_autoload_filename(load_filename, sizeof(load_filename)) == 0)
    {
      gpsp_boot_trace("gPSPDC boot: autoload load_gamepak");
      if(load_gamepak((char *)load_filename) == -1)
        gpsp_gamepak_load_error((char *)load_filename);

      gpsp_boot_trace("gPSPDC boot: autoload set resolution");
      set_gba_resolution(screen_scale);
      gpsp_boot_trace("gPSPDC boot: autoload init_cpu");
      init_cpu();
      gpsp_boot_trace("gPSPDC boot: autoload init_memory");
      init_memory();
      gpsp_dc_trace_loaded_rom("autoload");
    }
    else
#endif
    if(load_file(file_ext, load_filename) == -1)
    {
      gpsp_debug_printf("Loading menu...\n");
      {
        u16 *screen_copy = copy_screen();

        if(screen_copy == NULL)
          quit();

        menu(screen_copy);
        free(screen_copy);
      }
    }
    else
    {
      if(load_gamepak(load_filename) == -1)
      {
#ifdef _arch_dreamcast
        gpsp_gamepak_load_error((char *)load_filename);
#else
        printf("Failed to load gamepak %s, exiting.\n", load_filename);
        delay_us(5000000);
        exit(-1);
#endif
      }

      set_gba_resolution(screen_scale);
#ifndef _arch_dreamcast
      video_resolution_small();
#endif

      init_cpu();
      init_memory();
      gpsp_dc_trace_loaded_rom("menu load");
    }
  }

  last_frame = 0;

  if(gamepak_filename[0] == 0)
    quit();

  // We'll never actually return from here.

#if defined(PSP_BUILD) || (defined(_arch_dreamcast) && !defined(GPSP_DC_INTERPRETER))
  gpsp_boot_trace("gPSPDC boot: execute dynarec");
  execute_arm_translate(execute_cycles);
#else
  gpsp_boot_trace("gPSPDC boot: execute interpreter");
  execute_arm(execute_cycles);
#endif
  return 0;
}

void print_memory_stats(u32 *counter, u32 *region_stats, u8 *stats_str)
{
  u32 other_region_counter = region_stats[0x1] + region_stats[0xE] + region_stats[0xF];
  u32 rom_region_counter = region_stats[0x8] + region_stats[0x9] + region_stats[0xA] +
   region_stats[0xB] + region_stats[0xC] + region_stats[0xD];
  u32 _counter = *counter;

  printf("memory access stats: %s (out of %d)\n", stats_str, _counter);
  printf("bios: %f%%\tiwram: %f%%\tewram: %f%%\tvram: %f\n",
   region_stats[0x0] * 100.0 / _counter, region_stats[0x3] * 100.0 / _counter,
   region_stats[0x2] * 100.0 / _counter, region_stats[0x6] * 100.0 / _counter);

  printf("oam: %f%%\tpalette: %f%%\trom: %f%%\tother: %f%%\n",
   region_stats[0x7] * 100.0 / _counter, region_stats[0x5] * 100.0 / _counter,
   rom_region_counter * 100.0 / _counter, other_region_counter * 100.0 / _counter);

  *counter = 0;
  memset(region_stats, 0, sizeof(u32) * 16);
}

u32 update_gba()
{
  static u32 trace_update_gba;
  irq_type irq_raised = IRQ_NONE;
  cpu_ticks += execute_cycles;

  GPSP_DC_TRACE_EVERY(trace_update_gba,
   "update_gba #%u pc=%08x cpsr=%08x execute=%u video=%u frame=%u vcount=%u",
   trace_update_gba, reg[REG_PC], reg[REG_CPSR], execute_cycles,
   video_count, frame_ticks, io_registers[REG_VCOUNT]);
  gpsp_dc_trace_vram_dirty();

  if(gbc_sound_update)
  {
    gbc_update_count++;
    update_gbc_sound(cpu_ticks);
    gbc_sound_update = 0;
  }

  update_timer(0);
  update_timer(1);
  update_timer(2);
  update_timer(3);

  video_count -= execute_cycles;

  if(video_count <= 0)
  {
    u32 vcount = io_registers[REG_VCOUNT];
    u32 dispstat = io_registers[REG_DISPSTAT];

    if((dispstat & 0x02) == 0)
    {
      // Transition from hrefresh to hblank
      video_count += 272;
      dispstat |= 0x02;

      if((dispstat & 0x01) == 0)
      {
        u32 i;
        if(oam_update)
          oam_update_count++;

        update_scanline();


        // If in visible area also fire HDMA
        for(i = 0; i < 4; i++)
        {
          if(dma[i].start_type == DMA_START_HBLANK)
            gpsp_dma_transfer(dma + i);
        }
      }

      if(dispstat & 0x10)
        irq_raised |= IRQ_HBLANK;
    }
    else
    {
      // Transition from hblank to next line
      video_count += 960;
      dispstat &= ~0x02;

      vcount++;

      if(vcount == 160)
      {
        // Transition from vrefresh to vblank
        u32 i;

        dispstat |= 0x01;
        if(dispstat & 0x8)
        {
          irq_raised |= IRQ_VBLANK;
        }

        affine_reference_x[0] =
         (s32)(address32(io_registers, 0x28) << 4) >> 4;
        affine_reference_y[0] =
         (s32)(address32(io_registers, 0x2C) << 4) >> 4;
        affine_reference_x[1] =
         (s32)(address32(io_registers, 0x38) << 4) >> 4;
        affine_reference_y[1] =
         (s32)(address32(io_registers, 0x3C) << 4) >> 4;

        for(i = 0; i < 4; i++)
        {
          if(dma[i].start_type == DMA_START_VBLANK)
            gpsp_dma_transfer(dma + i);
        }
      }
      else

      if(vcount == 228)
      {
        // Transition from vblank to next screen
        dispstat &= ~0x01;
        frame_ticks++;

#ifdef PSP_BUILD
        printf("frame update (%x), %d instructions total, %d RAM flushes\n",
         reg[REG_PC], instruction_count - last_frame, flush_ram_count);
        last_frame = instruction_count;
        print_memory_stats(&memory_reads_u8, memory_region_access_read_u8,
         "unsigned 8bit read");
        print_memory_stats(&memory_reads_s8, memory_region_access_read_s8,
         "signed 8bit read");
        print_memory_stats(&memory_reads_u16, memory_region_access_read_u16,
         "unsigned 16bit read");
        print_memory_stats(&memory_reads_s16, memory_region_access_read_s16,
         "signed 16bit read");
        print_memory_stats(&memory_reads_u32, memory_region_access_read_u32,
         "32bit read");
        print_memory_stats(&memory_writes_u8, memory_region_access_write_u8,
         "8bit write");
        print_memory_stats(&memory_writes_u16, memory_region_access_write_u16,
         "16bit write");
        print_memory_stats(&memory_writes_u32, memory_region_access_write_u32,
         "32bit write");
        printf("%d gbc audio updates\n", gbc_update_count);
        printf("%d oam updates\n", oam_update_count); 
        gbc_update_count = 0;
        oam_update_count = 0;
        flush_ram_count = 0;
#endif

        {
          static u32 trace_update_input;
          static u32 trace_update_screen;

          GPSP_DC_TRACE_EVERY(trace_update_input,
           "update_input #%u pc=%08x frame=%u", trace_update_input,
           reg[REG_PC], frame_ticks);

          if(update_input())
            return execute_cycles;

          update_gbc_sound(cpu_ticks);
          synchronize();

          GPSP_DC_TRACE_EVERY(trace_update_screen,
           "update_screen #%u pc=%08x skip=%u", trace_update_screen,
           reg[REG_PC], skip_next_frame);
          update_screen();
        }

        if(update_backup_flag)
          update_backup();

        process_cheats();

        vcount = 0;
      }

      if(vcount == (dispstat >> 8))
      {
        // vcount trigger
        dispstat |= 0x04;
        if(dispstat & 0x20)
        {
          irq_raised |= IRQ_VCOUNT;
        }
      }
      else
      {
        dispstat &= ~0x04;
      }

      io_registers[REG_VCOUNT] = vcount;
    }
    io_registers[REG_DISPSTAT] = dispstat;
  }

  if(irq_raised)
    raise_interrupt(irq_raised);

  execute_cycles = video_count;

  check_timer(0);
  check_timer(1);
  check_timer(2);
  check_timer(3);

  return execute_cycles;
}

u64 last_screen_timestamp = 0;
u32 frame_speed = 15000;

#ifdef PSP_BUILD

u32 real_frame_count = 0;
u32 virtual_frame_count = 0;
u32 num_skipped_frames = 0;

void vblank_interrupt_handler(u32 sub, u32 *parg)
{
  real_frame_count++;
}

void synchronize()
{
  char char_buffer[64];
  u64 new_ticks, time_delta;
  s32 used_frameskip = frameskip_value;

  if(!synchronize_flag)
  {
    print_string("--FF--", 0xFFFF, 0x000, 0, 0);
    used_frameskip = 4;
    virtual_frame_count = real_frame_count - 1;
  }

  skip_next_frame = 0;

  virtual_frame_count++;

  if(real_frame_count >= virtual_frame_count)
  {
    if((real_frame_count > virtual_frame_count) &&
     (current_frameskip_type == auto_frameskip) &&
     (num_skipped_frames < frameskip_value))
    {
      skip_next_frame = 1;
      num_skipped_frames++;
    }
    else
    {
      virtual_frame_count = real_frame_count;
      num_skipped_frames = 0;
    }

    // Here so that the home button return will eventually work.
    // If it's not running fullspeed anyway this won't really hurt
    // it much more.

    delay_us(1);
  }
  else
  {
    if(synchronize_flag)
      sceDisplayWaitVblankStart();
  }

  if(current_frameskip_type == manual_frameskip)
  {
    frameskip_counter = (frameskip_counter + 1) %
     (used_frameskip + 1);
    if(random_skip)
    {
      if(frameskip_counter != (rand() % (used_frameskip + 1)))
        skip_next_frame = 1;
    }
    else
    {
      if(frameskip_counter)
        skip_next_frame = 1;
    }
  }

/*  sprintf(char_buffer, "%08d %08d %d %d %d\n",
   real_frame_count, virtual_frame_count, num_skipped_frames,
   real_frame_count - virtual_frame_count, skip_next_frame);
  print_string(char_buffer, 0xFFFF, 0x0000, 0, 10); */

/*
    sprintf(char_buffer, "%02d %02d %06d %07d", frameskip, (u32)ms_needed,
     ram_translation_ptr - ram_translation_cache, rom_translation_ptr -
     rom_translation_cache);
    print_string(char_buffer, 0xFFFF, 0x0000, 0, 0);
*/
}

#else

u32 ticks_needed_total = 0;
float us_needed = 0.0;
u32 frames = 0;
const u32 frame_interval = 60;

void synchronize()
{
  u64 new_ticks;
  u64 time_delta;
  char char_buffer[64];

  get_ticks_us(&new_ticks);
  time_delta = new_ticks - last_screen_timestamp;
  last_screen_timestamp = new_ticks;
  ticks_needed_total += time_delta;

  skip_next_frame = 0;

  if((time_delta < frame_speed) && synchronize_flag)
  {
    delay_us(frame_speed - time_delta);
  }

  frames++;

  if(frames == frame_interval)
  {
    us_needed = (float)ticks_needed_total / frame_interval;
    ticks_needed_total = 0;
    frames = 0;
  }

  if(current_frameskip_type == manual_frameskip)
  {
    frameskip_counter = (frameskip_counter + 1) %
     (frameskip_value + 1);
    if(random_skip)
    {
      if(frameskip_counter != (rand() % (frameskip_value + 1)))
        skip_next_frame = 1;
    }
    else
    {
      if(frameskip_counter)
        skip_next_frame = 1;
    }
  }

  if(synchronize_flag == 0)
    print_string("--FF--", 0xFFFF, 0x000, 0, 0);

  sprintf(char_buffer, "gpSP: %.1fms %.1ffps", us_needed / 1000.0,
   1000000.0 / us_needed);
  SDL_WM_SetCaption(char_buffer, "gpSP");

/*
    sprintf(char_buffer, "%02d %02d %06d %07d", frameskip, (u32)ms_needed,
     ram_translation_ptr - ram_translation_cache, rom_translation_ptr -
     rom_translation_cache);
    print_string(char_buffer, 0xFFFF, 0x0000, 0, 0);
*/
}

#endif

void quit()
{
  if(!update_backup_flag)
    update_backup_force();

  gpsp_finish_save_notice();
  sound_exit();

#ifdef PSP_BUILD
  sceKernelExitGame();
#else
  SDL_Quit();
  exit(0);
#endif
}

void reset_gba()
{
  init_main();
  init_memory();
  init_cpu();
  reset_sound();
}

#ifdef PSP_BUILD

u32 file_length(u8 *filename, s32 dummy)
{
  SceIoStat stats;
  sceIoGetstat(filename, &stats);
  return stats.st_size;
}

void delay_us(u32 us_count)
{
  sceKernelDelayThread(us_count);
}

void get_ticks_us(u64 *tick_return)
{
  u64 ticks;
  sceRtcGetCurrentTick(&ticks);

  *tick_return = (ticks * 1000000) / sceRtcGetTickResolution();
}

#else

u32 file_length(u8 *dummy, FILE *fp)
{
  u32 length;

  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  return length;
}

void delay_us(u32 us_count)
{
  SDL_Delay(us_count / 1000);
}

void get_ticks_us(u64 *ticks_return)
{
  *ticks_return = (SDL_GetTicks() * 1000);
}

#endif

void change_ext(u8 *src, u8 *buffer, u8 *extension)
{
  u8 *dot_position;
  strcpy(buffer, src);
  dot_position = strrchr(buffer, '.');

  if(dot_position)
    strcpy(dot_position, extension);
}

#define main_savestate_builder(type)                                          \
void main_##type##_savestate(file_tag_type savestate_file)                    \
{                                                                             \
  file_##type##_variable(savestate_file, cpu_ticks);                          \
  file_##type##_variable(savestate_file, execute_cycles);                     \
  file_##type##_variable(savestate_file, video_count);                        \
  file_##type##_array(savestate_file, timer);                                 \
}                                                                             \

main_savestate_builder(read);
main_savestate_builder(write_mem);

void print_out(u32 address, u32 pc)
{
  char buffer[256];
  sprintf(buffer, "patching from gp8 %x", address);
  print_string(buffer, 0xFFFF, 0x0000, 0, 0);
  update_screen();
  delay_us(5000000);
}
