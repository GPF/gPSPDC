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
u32 cheat_master_hook = 0xffffffff;

/* PAR v3 / Gameshark v3 cheat decoding adapted from SkyEmu (MIT License).
   See THIRD_PARTY_NOTICES.md */

#define PAR3_IF_STACK_MAX 32

static void cheat_set_master_hook(u32 pcaddr)
{
  if(cheat_master_hook != pcaddr)
  {
    cheat_master_hook = pcaddr;
    flush_translation_cache_rom();
    flush_translation_cache_ram();
    flush_translation_cache_bios();
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

void decrypt_gsa_code(u32 *address_ptr, u32 *value_ptr, cheat_variant_enum
 cheat_variant)
{
  u32 i, i2, code_position;
  u32 address = *address_ptr;
  u32 value = *value_ptr;
  u32 r = 0xc6ef3720;

  u32 seeds_v1[4] =
  {
    0x09f4fbbd, 0x9681884a, 0x352027e9, 0xf3dee5a7
  };
  u32 seeds_v3[4] =
  {
    0x7aa9648f, 0x7fae6994, 0xc0efaad5, 0x42712c57
  };
  u32 *seeds;

  if(cheat_variant == CHEAT_TYPE_GAMESHARK_V1)
    seeds = seeds_v1;
  else
    seeds = seeds_v3;

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
  cheat_master_hook = 0xffffffff;
  #ifdef _arch_dreamcast
  // add /cd/gbaDC/ to the cheats_filename path
  u8 cheats_filename2[512];
  sprintf(cheats_filename2, "/cd/gbaDC/%s", cheats_filename);
  cheats_file = fopen(cheats_filename2, "rb"); 
#else
  cheats_file = fopen(cheats_filename, "rb");
#endif
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

        while(fgets(current_line, 256, cheats_file))
        {
          if(strlen(current_line) < 3)
            break;

          sscanf(current_line, "%08x %08x", &address, &value);

          decrypt_gsa_code(&address, &value, current_cheat_variant);

          cheat_code_ptr[0] = address;
          cheat_code_ptr[1] = value;

          cheat_code_ptr += 2;
          num_cheat_lines++;
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
      cheat_set_master_hook(0x08000000 | (address & 0x1FFFFFF));
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
        cheat_set_master_hook(0x08000000 | (address & 0x1FFFFFF));
        break;
    }
  }
}

void process_cheat_gs3(cheat_type *cheat)
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
      cheat_set_master_hook(0x08000000 | (left & 0x1FFFFFF));
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
          cheat_set_master_hook(0x08000000 | (address & 0x1FFFFFE));
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
