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
#include "cpu.h"

cheat_type cheats[MAX_CHEATS];
u32 num_cheats;
u32 cheat_master_hooks[MAX_CHEAT_HOOKS];
u32 cheat_num_master_hooks;
u32 cheat_master_hook = 0xffffffff;

/* PAR v3 / Gameshark v3 cheat decoding adapted from SkyEmu (MIT License).
   DEADFACE reseed tables from mGBA (MPL 2.0). See THIRD_PARTY_NOTICES.md */

#define PAR3_IF_STACK_MAX 32

/* mGBA gameshark.c / parv3.c reseed tables (MPL 2.0) */
static const u8 _gsa1T1[256] = {
  0x31, 0x1C, 0x23, 0xE5, 0x89, 0x8E, 0xA1, 0x37, 0x74, 0x6D, 0x67, 0xFC, 0x1F, 0xC0, 0xB1, 0x94,
  0x3B, 0x05, 0x56, 0x86, 0x00, 0x24, 0xF0, 0x17, 0x72, 0xA2, 0x3D, 0x1B, 0xE3, 0x17, 0xC5, 0x0B,
  0xB9, 0xE2, 0xBD, 0x58, 0x71, 0x1B, 0x2C, 0xFF, 0xE4, 0xC9, 0x4C, 0x5E, 0xC9, 0x55, 0x33, 0x45,
  0x7C, 0x3F, 0xB2, 0x51, 0xFE, 0x10, 0x7E, 0x75, 0x3C, 0x90, 0x8D, 0xDA, 0x94, 0x38, 0xC3, 0xE9,
  0x95, 0xEA, 0xCE, 0xA6, 0x06, 0xE0, 0x4F, 0x3F, 0x2A, 0xE3, 0x3A, 0xE4, 0x43, 0xBD, 0x7F, 0xDA,
  0x55, 0xF0, 0xEA, 0xCB, 0x2C, 0xA8, 0x47, 0x61, 0xA0, 0xEF, 0xCB, 0x13, 0x18, 0x20, 0xAF, 0x3E,
  0x4D, 0x9E, 0x1E, 0x77, 0x51, 0xC5, 0x51, 0x20, 0xCF, 0x21, 0xF9, 0x39, 0x94, 0xDE, 0xDD, 0x79,
  0x4E, 0x80, 0xC4, 0x9D, 0x94, 0xD5, 0x95, 0x01, 0x27, 0x27, 0xBD, 0x6D, 0x78, 0xB5, 0xD1, 0x31,
  0x6A, 0x65, 0x74, 0x74, 0x58, 0xB3, 0x7C, 0xC9, 0x5A, 0xED, 0x50, 0x03, 0xC4, 0xA2, 0x94, 0x4B,
  0xF0, 0x58, 0x09, 0x6F, 0x3E, 0x7D, 0xAE, 0x7D, 0x58, 0xA0, 0x2C, 0x91, 0xBB, 0xE1, 0x70, 0xEB,
  0x73, 0xA6, 0x9A, 0x44, 0x25, 0x90, 0x16, 0x62, 0x53, 0xAE, 0x08, 0xEB, 0xDC, 0xF0, 0xEE, 0x77,
  0xC2, 0xDE, 0x81, 0xE8, 0x30, 0x89, 0xDB, 0xFE, 0xBC, 0xC2, 0xDF, 0x26, 0xE9, 0x8B, 0xD6, 0x93,
  0xF0, 0xCB, 0x56, 0x90, 0xC0, 0x46, 0x68, 0x15, 0x43, 0xCB, 0xE9, 0x98, 0xE3, 0xAF, 0x31, 0x25,
  0x4D, 0x7B, 0xF3, 0xB1, 0x74, 0xE2, 0x64, 0xAC, 0xD9, 0xF6, 0xA0, 0xD5, 0x0B, 0x9B, 0x49, 0x52,
  0x69, 0x3B, 0x71, 0x00, 0x2F, 0xBB, 0xBA, 0x08, 0xB1, 0xAE, 0xBB, 0xB3, 0xE1, 0xC9, 0xA6, 0x7F,
  0x17, 0x97, 0x28, 0x72, 0x12, 0x6E, 0x91, 0xAE, 0x3A, 0xA2, 0x35, 0x46, 0x27, 0xF8, 0x12, 0x50
};

static const u8 _gsa1T2[256] = {
  0xD8, 0x65, 0x04, 0xC2, 0x65, 0xD5, 0xB0, 0x0C, 0xDF, 0x9D, 0xF0, 0xC3, 0x9A, 0x17, 0xC9, 0xA6,
  0xE1, 0xAC, 0x0D, 0x14, 0x2F, 0x3C, 0x2C, 0x87, 0xA2, 0xBF, 0x4D, 0x5F, 0xAC, 0x2D, 0x9D, 0xE1,
  0x0C, 0x9C, 0xE7, 0x7F, 0xFC, 0xA8, 0x66, 0x59, 0xAC, 0x18, 0xD7, 0x05, 0xF0, 0xBF, 0xD1, 0x8B,
  0x35, 0x9F, 0x59, 0xB4, 0xBA, 0x55, 0xB2, 0x85, 0xFD, 0xB1, 0x72, 0x06, 0x73, 0xA4, 0xDB, 0x48,
  0x7B, 0x5F, 0x67, 0xA5, 0x95, 0xB9, 0xA5, 0x4A, 0xCF, 0xD1, 0x44, 0xF3, 0x81, 0xF5, 0x6D, 0xF6,
  0x3A, 0xC3, 0x57, 0x83, 0xFA, 0x8E, 0x15, 0x2A, 0xA2, 0x04, 0xB2, 0x9D, 0xA8, 0x0D, 0x7F, 0xB8,
  0x0F, 0xF6, 0xAC, 0xBE, 0x97, 0xCE, 0x16, 0xE6, 0x31, 0x10, 0x60, 0x16, 0xB5, 0x83, 0x45, 0xEE,
  0xD7, 0x5F, 0x2C, 0x08, 0x58, 0xB1, 0xFD, 0x7E, 0x79, 0x00, 0x34, 0xAD, 0xB5, 0x31, 0x34, 0x39,
  0xAF, 0xA8, 0xDD, 0x52, 0x6A, 0xB0, 0x60, 0x35, 0xB8, 0x1D, 0x52, 0xF5, 0xF5, 0x30, 0x00, 0x7B,
  0xF4, 0xBA, 0x03, 0xCB, 0x3A, 0x84, 0x14, 0x8A, 0x6A, 0xEF, 0x21, 0xBD, 0x01, 0xD8, 0xA0, 0xD4,
  0x43, 0xBE, 0x23, 0xE7, 0x76, 0x27, 0x2C, 0x3F, 0x4D, 0x3F, 0x43, 0x18, 0xA7, 0xC3, 0x47, 0xA5,
  0x7A, 0x1D, 0x02, 0x55, 0x09, 0xD1, 0xFF, 0x55, 0x5E, 0x17, 0xA0, 0x56, 0xF4, 0xC9, 0x6B, 0x90,
  0xB4, 0x80, 0xA5, 0x07, 0x22, 0xFB, 0x22, 0x0D, 0xD9, 0xC0, 0x5B, 0x08, 0x35, 0x05, 0xC1, 0x75,
  0x4F, 0xD0, 0x51, 0x2D, 0x2E, 0x5E, 0x69, 0xE7, 0x3B, 0xC2, 0xDA, 0xFF, 0xF6, 0xCE, 0x3E, 0x76,
  0xE8, 0x36, 0x8C, 0x39, 0xD8, 0xF3, 0xE9, 0xA6, 0x42, 0xE6, 0xC1, 0x4C, 0x05, 0xBE, 0x17, 0xF2,
  0x5C, 0x1B, 0x19, 0xDB, 0x0F, 0xF3, 0xF8, 0x49, 0xEB, 0x36, 0xF6, 0x40, 0x6F, 0xAD, 0xC1, 0x8C
};

static const u8 _par3T1[256] = {
  0xD0, 0xFF, 0xBA, 0xE5, 0xC1, 0xC7, 0xDB, 0x5B, 0x16, 0xE3, 0x6E, 0x26, 0x62, 0x31, 0x2E, 0x2A,
  0xD1, 0xBB, 0x4A, 0xE6, 0xAE, 0x2F, 0x0A, 0x90, 0x29, 0x90, 0xB6, 0x67, 0x58, 0x2A, 0xB4, 0x45,
  0x7B, 0xCB, 0xF0, 0x73, 0x84, 0x30, 0x81, 0xC2, 0xD7, 0xBE, 0x89, 0xD7, 0x4E, 0x73, 0x5C, 0xC7,
  0x80, 0x1B, 0xE5, 0xE4, 0x43, 0xC7, 0x46, 0xD6, 0x6F, 0x7B, 0xBF, 0xED, 0xE5, 0x27, 0xD1, 0xB5,
  0xD0, 0xD8, 0xA3, 0xCB, 0x2B, 0x30, 0xA4, 0xF0, 0x84, 0x14, 0x72, 0x5C, 0xFF, 0xA4, 0xFB, 0x54,
  0x9D, 0x70, 0xE2, 0xFF, 0xBE, 0xE8, 0x24, 0x76, 0xE5, 0x15, 0xFB, 0x1A, 0xBC, 0x87, 0x02, 0x2A,
  0x58, 0x8F, 0x9A, 0x95, 0xBD, 0xAE, 0x8D, 0x0C, 0xA5, 0x4C, 0xF2, 0x5C, 0x7D, 0xAD, 0x51, 0xFB,
  0xB1, 0x22, 0x07, 0xE0, 0x29, 0x7C, 0xEB, 0x98, 0x14, 0xC6, 0x31, 0x97, 0xE4, 0x34, 0x8F, 0xCC,
  0x99, 0x56, 0x9F, 0x78, 0x43, 0x91, 0x85, 0x3F, 0xC2, 0xD0, 0xD1, 0x80, 0xD1, 0x77, 0xA7, 0xE2,
  0x43, 0x99, 0x1D, 0x2F, 0x8B, 0x6A, 0xE4, 0x66, 0x82, 0xF7, 0x2B, 0x0B, 0x65, 0x14, 0xC0, 0xC2,
  0x1D, 0x96, 0x78, 0x1C, 0xC4, 0xC3, 0xD2, 0xB1, 0x64, 0x07, 0xD7, 0x6F, 0x02, 0xE9, 0x44, 0x31,
  0xDB, 0x3C, 0xEB, 0x93, 0xED, 0x9A, 0x57, 0x05, 0xB9, 0x0E, 0xAF, 0x1F, 0x48, 0x11, 0xDC, 0x35,
  0x6C, 0xB8, 0xEE, 0x2A, 0x48, 0x2B, 0xBC, 0x89, 0x12, 0x59, 0xCB, 0xD1, 0x18, 0xEA, 0x72, 0x11,
  0x01, 0x75, 0x3B, 0xB5, 0x56, 0xF4, 0x8B, 0xA0, 0x41, 0x75, 0x86, 0x7B, 0x94, 0x12, 0x2D, 0x4C,
  0x0C, 0x22, 0xC9, 0x4A, 0xD8, 0xB1, 0x8D, 0xF0, 0x55, 0x2E, 0x77, 0x50, 0x1C, 0x64, 0x77, 0xAA,
  0x3E, 0xAC, 0xD3, 0x3D, 0xCE, 0x60, 0xCA, 0x5D, 0xA0, 0x92, 0x78, 0xC6, 0x51, 0xFE, 0xF9, 0x30
};

static const u8 _par3T2[256] = {
  0xAA, 0xAF, 0xF0, 0x72, 0x90, 0xF7, 0x71, 0x27, 0x06, 0x11, 0xEB, 0x9C, 0x37, 0x12, 0x72, 0xAA,
  0x65, 0xBC, 0x0D, 0x4A, 0x76, 0xF6, 0x5C, 0xAA, 0xB0, 0x7A, 0x7D, 0x81, 0xC1, 0xCE, 0x2F, 0x9F,
  0x02, 0x75, 0x38, 0xC8, 0xFC, 0x66, 0x05, 0xC2, 0x2C, 0xBD, 0x91, 0xAD, 0x03, 0xB1, 0x88, 0x93,
  0x31, 0xC6, 0xAB, 0x40, 0x23, 0x43, 0x76, 0x54, 0xCA, 0xE7, 0x00, 0x96, 0x9F, 0xD8, 0x24, 0x8B,
  0xE4, 0xDC, 0xDE, 0x48, 0x2C, 0xCB, 0xF7, 0x84, 0x1D, 0x45, 0xE5, 0xF1, 0x75, 0xA0, 0xED, 0xCD,
  0x4B, 0x24, 0x8A, 0xB3, 0x98, 0x7B, 0x12, 0xB8, 0xF5, 0x63, 0x97, 0xB3, 0xA6, 0xA6, 0x0B, 0xDC,
  0xD8, 0x4C, 0xA8, 0x99, 0x27, 0x0F, 0x8F, 0x94, 0x63, 0x0F, 0xB0, 0x11, 0x94, 0xC7, 0xE9, 0x7F,
  0x3B, 0x40, 0x72, 0x4C, 0xDB, 0x84, 0x78, 0xFE, 0xB8, 0x56, 0x08, 0x80, 0xDF, 0x20, 0x2F, 0xB9,
  0x66, 0x2D, 0x60, 0x63, 0xF5, 0x18, 0x15, 0x1B, 0x86, 0x85, 0xB9, 0xB4, 0x68, 0x0E, 0xC6, 0xD1,
  0x8A, 0x81, 0x2B, 0xB3, 0xF6, 0x48, 0xF0, 0x4F, 0x9C, 0x28, 0x1C, 0xA4, 0x51, 0x2F, 0xD7, 0x4B,
  0x17, 0xE7, 0xCC, 0x50, 0x9F, 0xD0, 0xD1, 0x40, 0x0C, 0x0D, 0xCA, 0x83, 0xFA, 0x5E, 0xCA, 0xEC,
  0xBF, 0x4E, 0x7C, 0x8F, 0xF0, 0xAE, 0xC2, 0xD3, 0x28, 0x41, 0x9B, 0xC8, 0x04, 0xB9, 0x4A, 0xBA,
  0x72, 0xE2, 0xB5, 0x06, 0x2C, 0x1E, 0x0B, 0x2C, 0x7F, 0x11, 0xA9, 0x26, 0x51, 0x9D, 0x3F, 0xF8,
  0x62, 0x11, 0x2E, 0x89, 0xD2, 0x9D, 0x35, 0xB1, 0xE4, 0x0A, 0x4D, 0x93, 0x01, 0xA7, 0xD1, 0x2D,
  0x00, 0x87, 0xE2, 0x2D, 0xA4, 0xE9, 0x0A, 0x06, 0x66, 0xF8, 0x1F, 0x44, 0x75, 0xB5, 0x6B, 0x1C,
  0xFC, 0x31, 0x09, 0x48, 0xA3, 0xFF, 0x92, 0x12, 0x58, 0xE9, 0xFA, 0xAE, 0x4F, 0xE2, 0xB4, 0xCC
};



static const u32 gsa1_seeds_default[4] =
{
  0x09f4fbbd, 0x9681884a, 0x352027e9, 0xf3dee5a7
};

static const u32 par3_seeds_default[4] =
{
  0x7aa9648f, 0x7fae6994, 0xc0efaad5, 0x42712c57
};

static void cheat_reset_master_hooks(void)
{
  cheat_num_master_hooks = 0;
  cheat_master_hook = 0xffffffff;
}

static void cheat_flush_translation_caches(void)
{
  flush_translation_cache_rom();
  flush_translation_cache_ram();
  flush_translation_cache_bios();
}

static u32 cheat_hook_pc_valid(u32 pcaddr)
{
  if(pcaddr & 0x01)
    return 0;

  if((pcaddr & 0xFF000000) == 0x08000000)
    return 1;

  if((pcaddr & 0xFF000000) == 0x02000000)
    return 1;

  if((pcaddr & 0xFFFF0000) == 0x03000000)
    return 1;

  return 0;
}

static void cheat_add_master_hook(u32 pcaddr)
{
  u32 i;

  if(!cheat_hook_pc_valid(pcaddr))
    return;

  for(i = 0; i < cheat_num_master_hooks; i++)
  {
    if(cheat_master_hooks[i] == pcaddr)
      return;
  }

  if(cheat_num_master_hooks >= MAX_CHEAT_HOOKS)
    return;

  cheat_master_hooks[cheat_num_master_hooks++] = pcaddr;
  if(cheat_num_master_hooks == 1)
    cheat_master_hook = pcaddr;

  cheat_flush_translation_caches();
}

u32 cheat_pc_is_hook(u32 pc)
{
  u32 i;

  for(i = 0; i < cheat_num_master_hooks; i++)
  {
    if(cheat_master_hooks[i] == pc)
      return 1;
  }

  return 0;
}

static void cheat_reseed(u32 *seeds, u16 params, const u8 *t1, const u8 *t2)
{
  s32 x, y;
  s32 s0 = params >> 8;
  s32 s1 = params & 0xFF;

  for(y = 0; y < 4; y++)
  {
    for(x = 0; x < 4; x++)
    {
      u8 z = t1[(s0 + x) & 0xFF] + t2[(s1 + y) & 0xFF];
      seeds[y] <<= 8;
      seeds[y] |= z;
    }
  }
}

static u32 par3_ar_address(u32 left)
{
  return ((left << 4) & 0x0F000000) | (left & 0x000FFFFF);
}

static u32 par3_handle_ar_if(u32 left, u32 right)
{
  u32 address = par3_ar_address(left);
  u32 current_code = (left >> 24) & 0xFF;
  u32 left_compare = read_memory32(address);
  u32 right_compare = right;
  s32 left_signed;
  s32 right_signed;

  switch(current_code & 0x06)
  {
    case 0x00:
      left_compare &= 0xFF;
      right_compare &= 0xFF;
      left_signed = (s8)left_compare;
      right_signed = (s8)right_compare;
      break;

    case 0x02:
      left_compare &= 0xFFFF;
      right_compare &= 0xFFFF;
      left_signed = (s16)left_compare;
      right_signed = (s16)right_compare;
      break;

    case 0x04:
      left_signed = (s32)left_compare;
      right_signed = (s32)right_compare;
      break;

    default:
      return 0;
  }

  switch(current_code & 0x38)
  {
    case 0x08:
      return left_compare == right_compare;

    case 0x10:
      return left_compare != right_compare;

    case 0x18:
      return left_signed < right_signed;

    case 0x20:
      return left_signed > right_signed;

    case 0x28:
      return left_compare < right_compare;

    case 0x30:
      return left_compare > right_compare;

    case 0x38:
      return left_compare && right_compare;

    default:
      return 0;
  }
}

static u32 gs1_if_pass(u32 cond, u32 memval, u32 operand)
{
  switch(cond)
  {
    case 0:
      return memval == operand;

    case 1:
      return memval != operand;

    case 2:
      return (s16)memval <= (s16)operand;

    case 3:
      return (s16)memval >= (s16)operand;

    default:
      return 0;
  }
}

static void decrypt_gsa_code_seeds(u32 *address_ptr, u32 *value_ptr,
 const u32 *seeds)
{
  u32 i;
  u32 address = *address_ptr;
  u32 value = *value_ptr;
  u32 r = 0xc6ef3720;

  for(i = 0; i < 32; i++)
  {
    value -= ((address << 4) + seeds[2]) ^ (address + r) ^
     ((address >> 5) + seeds[3]);
    address -= ((value << 4) + seeds[0]) ^ (value + r) ^
     ((value >> 5) + seeds[1]);
    r -= 0x9e3779b9;
  }

  *address_ptr = address;
  *value_ptr = value;
}

void decrypt_gsa_code(u32 *address_ptr, u32 *value_ptr, cheat_variant_enum
 cheat_variant)
{
  if(cheat_variant == CHEAT_TYPE_GAMESHARK_V1)
    decrypt_gsa_code_seeds(address_ptr, value_ptr, gsa1_seeds_default);
  else
    decrypt_gsa_code_seeds(address_ptr, value_ptr, par3_seeds_default);
}

void add_cheats(u8 *cheats_filename)
{
  FILE *cheats_file;
  u8 current_line[256];
  u8 *name_ptr;
  u32 *cheat_code_ptr;
  u32 address, value;
  u32 num_cheat_lines;
  u32 cheat_name_length;
  cheat_variant_enum current_cheat_variant;

  num_cheats = 0;
  cheat_reset_master_hooks();
  cheats_file = fopen(cheats_filename, "rb");
  if(cheats_file)
  {
    while(fgets(current_line, 256, cheats_file))
    {
      // Get the header line first
      name_ptr = strchr(current_line, ' ');
      if(name_ptr)
      {
        *name_ptr = 0;
        name_ptr++;
      }

      if(!strcasecmp(current_line, "gameshark_v1") ||
       !strcasecmp(current_line, "gameshark_v2") ||
       !strcasecmp(current_line, "PAR_v1") ||
       !strcasecmp(current_line, "PAR_v2"))
      {
        current_cheat_variant = CHEAT_TYPE_GAMESHARK_V1;
      }
      else

      if(!strcasecmp(current_line, "gameshark_v3") ||
       !strcasecmp(current_line, "PAR_v3"))
      {
        current_cheat_variant = CHEAT_TYPE_GAMESHARK_V3;
      }
      else
      {
        current_cheat_variant = CHEAT_TYPE_INVALID;
      }

      if(current_cheat_variant != CHEAT_TYPE_INVALID)
      {
        strncpy(cheats[num_cheats].cheat_name, name_ptr, CHEAT_NAME_LENGTH - 1);
        cheats[num_cheats].cheat_name[CHEAT_NAME_LENGTH - 1] = 0;
        cheat_name_length = strlen(cheats[num_cheats].cheat_name);
        if (cheat_name_length > 0 &&
          ((cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\n') ||
           (cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\r')))
      {
          cheats[num_cheats].cheat_name[cheat_name_length - 1] = 0;
          cheat_name_length--;
      }
      

        if(cheat_name_length &&
         cheats[num_cheats].cheat_name[cheat_name_length - 1] == '\r')
        {
          cheats[num_cheats].cheat_name[cheat_name_length - 1] = 0;
        }

        cheats[num_cheats].cheat_variant = current_cheat_variant;
        cheat_code_ptr = cheats[num_cheats].cheat_codes;
        num_cheat_lines = 0;
        {
          u32 seeds[4];

          if(current_cheat_variant == CHEAT_TYPE_GAMESHARK_V1)
            memcpy(seeds, gsa1_seeds_default, sizeof(seeds));
          else
            memcpy(seeds, par3_seeds_default, sizeof(seeds));

          while(fgets(current_line, 256, cheats_file))
          {
            if(strlen(current_line) < 3)
              break;

            sscanf(current_line, "%08x %08x", &address, &value);

            decrypt_gsa_code_seeds(&address, &value, seeds);

            if(address == 0xDEADFACE)
            {
              if(current_cheat_variant == CHEAT_TYPE_GAMESHARK_V1)
                cheat_reseed(seeds, value & 0xFFFF, _gsa1T1, _gsa1T2);
              else
                cheat_reseed(seeds, value & 0xFFFF, _par3T1, _par3T2);
              continue;
            }

            cheat_code_ptr[0] = address;
            cheat_code_ptr[1] = value;

            cheat_code_ptr += 2;
            num_cheat_lines++;
          }
        }

        cheats[num_cheats].num_cheat_lines = num_cheat_lines;

        num_cheats++;
      }
    }

    fclose(cheats_file);
  }
}

void process_cheat_gs1(cheat_type *cheat)
{
  u32 cheat_opcode;
  u32 *code_ptr = cheat->cheat_codes;
  u32 address, value;
  u32 i;

  for(i = 0; i < cheat->num_cheat_lines; i++)
  {
    address = code_ptr[0];
    value = code_ptr[1];

    code_ptr += 2;

    if(value == 0x001DC0DE)
    {
      cheat_add_master_hook(0x08000000 | (address & 0x1FFFFFF));
      continue;
    }

    cheat_opcode = address >> 28;
    address &= 0xFFFFFFF;

    switch(cheat_opcode)
    {
      case 0x0:
        write_memory8(address, value);
        break;

      case 0x1:
        write_memory16(address, value);
        break;

      case 0x2:
        write_memory32(address, value);
        break;

      case 0x3:
      {
        u32 num_addresses = address & 0xFFFF;
        u32 address1, address2;
        u32 i2;

        for(i2 = 0; i2 < num_addresses; i2++)
        {
          address1 = code_ptr[0];
          address2 = code_ptr[1];
          code_ptr += 2;
          i++;

          write_memory32(address1, value);
          if(address2 != 0)
            write_memory32(address2, value);
        }
        break;
      }

      case 0x6:
        write_memory32(0x08000000 | (address & 0x1FFFFFF), value);
        break;

      case 0x8:
      {
        u32 pad = (~io_registers[REG_P1]) & 0x3FF;
        u32 button_mask = (value >> 16) & 0x3FF;

        if(button_mask == 0 || (pad & button_mask) == button_mask)
          write_memory32(address, value & 0xFFFF);
        break;
      }

      case 0xD:
        if(address == 0xDEADFACE)
          break;

        if(!gs1_if_pass((value >> 20) & 0x0F, read_memory16(address),
         value & 0xFFFF))
        {
          code_ptr += 2;
          i++;
        }
        break;

      case 0xE:
      {
        u32 check_addr = value & 0x0FFFFFFF;
        u32 operand = address & 0xFFFF;
        u32 skip = (address >> 16) & 0xFF;

        if(read_memory16(check_addr) != operand)
        {
          code_ptr += skip * 2;
          i += skip;
        }
        break;
      }

      case 0x0F:
        cheat_add_master_hook(0x08000000 | (address & 0x1FFFFFF));
        break;
    }
  }
}

static u32 par3_slowdown_loops(cheat_type *cheat)
{
  u32 i;
  u32 loops = 1;

  for(i = 0; i < cheat->num_cheat_lines; i++)
  {
    u32 left = cheat->cheat_codes[i * 2];
    u32 right = cheat->cheat_codes[i * 2 + 1];

    if(left == 0 && (right & 0xFF000000) == 0x08000000)
    {
      u32 factor = (right >> 8) & 0xFF;

      if(factor > loops)
        loops = factor;
    }
  }

  return loops ? loops : 1;
}

static void process_cheat_gs3_once(cheat_type *cheat)
{
  u32 if_stack[PAR3_IF_STACK_MAX];
  u32 if_stack_index = 0;
  u32 *codes = cheat->cheat_codes;
  u32 num_lines = cheat->num_cheat_lines;
  u32 i;
  u32 pad = (~io_registers[REG_P1]) & 0x3FF;

  if_stack[0] = 1;

  for(i = 0; i < num_lines; i++)
  {
    u32 left = codes[i * 2];
    u32 right = codes[i * 2 + 1];
    u32 current_code;
    u32 address;

    if(!if_stack[if_stack_index])
      continue;

    if(right == 0x001DC0DE)
    {
      cheat_add_master_hook(0x08000000 | (left & 0x1FFFFFF));
      continue;
    }

    if(left != 0)
    {
      current_code = (left >> 24) & 0xFF;
      address = par3_ar_address(left);

      switch(current_code)
      {
        case 0x00:
        {
          u32 offset = right >> 8;
          write_memory8(address + offset, right & 0xFF);
          break;
        }

        case 0x02:
        {
          u32 offset = right >> 16;
          write_memory16(address + (offset * 2), right & 0xFFFF);
          break;
        }

        case 0x04:
          write_memory32(address, right);
          break;

        case 0x40:
        {
          u32 offset = right >> 8;
          address = read_memory32(address);
          write_memory8(address + offset, right & 0xFF);
          break;
        }

        case 0x42:
        {
          u32 offset = right >> 16;
          address = read_memory32(address);
          write_memory16(address + (offset * 2), right & 0xFFFF);
          break;
        }

        case 0x44:
          address = read_memory32(address);
          write_memory32(address, right);
          break;

        case 0x80:
        {
          u8 data = right & 0xFF;
          write_memory8(address, read_memory8(address) + data);
          break;
        }

        case 0x82:
        {
          u16 data = right & 0xFFFF;
          write_memory16(address, read_memory16(address) + data);
          break;
        }

        case 0x84:
          write_memory32(address, read_memory32(address) + right);
          break;

        case 0xC4:
          cheat_add_master_hook(0x08000000 | (address & 0x1FFFFFE));
          break;

        case 0xC6:
          write_memory16(0x04000000 | (left & 0xFFFFFF), right & 0xFFFF);
          break;

        case 0xC7:
          write_memory32(0x04000000 | (left & 0xFFFFFF), right);
          break;

        default:
        {
          u32 condition = par3_handle_ar_if(left, right);

          switch(current_code & 0xC0)
          {
            case 0x00:
              if(!condition)
              {
                i++;
                continue;
              }
              /* fall through */

            case 0x40:
              if(!condition)
              {
                i += 2;
                continue;
              }
              break;

            case 0x80:
              break;

            case 0xC0:
              if(!condition)
                return;
              break;
          }

          if(if_stack_index + 1 >= PAR3_IF_STACK_MAX)
            return;

          if_stack_index++;
          if_stack[if_stack_index] = condition;
          break;
        }
      }
    }
    else
    {
      current_code = (right >> 24) & 0xFF;

      if(right == 0)
        return;

      switch(current_code)
      {
        case 0x60:
          if_stack[if_stack_index] ^= 1;
          break;

        case 0x40:
          if(if_stack_index == 0)
            return;
          if_stack_index--;
          break;

        case 0x08:
          break;

        case 0x18:
        case 0x1A:
        case 0x1C:
        case 0x1E:
          if(i + 1 >= num_lines)
            return;
          address = 0x08000000 | ((right & 0xFFFFFF) << 1);
          write_memory16(address, codes[(i + 1) * 2] & 0xFFFF);
          i++;
          break;

        case 0x10:
          if(i + 1 >= num_lines)
            return;
          if(pad != 0x3FF)
          {
            address = par3_ar_address(right);
            write_memory8(address, codes[(i + 1) * 2] & 0xFF);
          }
          i++;
          break;

        case 0x12:
          if(i + 1 >= num_lines)
            return;
          if(pad != 0x3FF)
          {
            address = par3_ar_address(right);
            write_memory16(address, codes[(i + 1) * 2] & 0xFFFF);
          }
          i++;
          break;

        case 0x14:
          if(i + 1 >= num_lines)
            return;
          if(pad != 0x3FF)
          {
            address = par3_ar_address(right);
            write_memory32(address, codes[(i + 1) * 2]);
          }
          i++;
          break;

        case 0x80:
        case 0x82:
        case 0x84:
        {
          u32 next_left, next_right;
          u32 repeat, data_increment, address_increment, data, j;

          if(i + 1 >= num_lines)
            return;

          next_left = codes[(i + 1) * 2];
          next_right = codes[(i + 1) * 2 + 1];
          address = par3_ar_address(right);
          repeat = (next_right >> 16) & 0xFF;
          data_increment = (next_right >> 24) & 0xFF;
          address_increment = next_right & 0xFFFF;
          data = next_left;

          if((current_code & 0x0F) == 0x02)
            address_increment *= 2;
          else
          if((current_code & 0x0F) == 0x04)
            address_increment *= 4;

          for(j = 0; j < repeat; j++)
          {
            if((current_code & 0x0F) == 0x02)
              write_memory16(address, data & 0xFFFF);
            else
            if((current_code & 0x0F) == 0x04)
              write_memory32(address, data);
            else
              write_memory8(address, data & 0xFF);

            address += address_increment;
            data += data_increment;
          }

          i++;
          break;
        }

        default:
          break;
      }
    }
  }
}

void process_cheat_gs3(cheat_type *cheat)
{
  u32 loops = par3_slowdown_loops(cheat);
  u32 i;

  for(i = 0; i < loops; i++)
    process_cheat_gs3_once(cheat);
}



void process_cheats()
{
  u32 i;

  for(i = 0; i < num_cheats; i++)
  {
    if(cheats[i].cheat_active)
    {
      switch(cheats[i].cheat_variant)
      {
        case CHEAT_TYPE_GAMESHARK_V1:
          process_cheat_gs1(cheats + i);
          break;

        case CHEAT_TYPE_GAMESHARK_V3:
          process_cheat_gs3(cheats + i);
          break;
      }
    }
  }
}
