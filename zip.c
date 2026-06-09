/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2006 SiberianSTAR
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

#include <zlib.h>
#include "common.h"

#define ZIP_BUFFER_SIZE (128 * 1024)

struct SZIPFileDataDescriptor
{
  s32 CRC32;
  s32 CompressedSize;
  s32 UncompressedSize;
} __attribute__((packed));

struct SZIPFileHeader
{
  s32 Sig;
  s16 VersionToExtract;
  s16 GeneralBitFlag;
  s16 CompressionMethod;
  s16 LastModFileTime;
  s16 LastModFileDate;
  struct SZIPFileDataDescriptor DataDescriptor;
  s16 FilenameLength;
  s16 ExtraFieldLength;
}  __attribute__((packed));

u32 load_file_zip(char *filename)
{
  struct SZIPFileHeader data;
  u8 tmp[1024];
  s32 retval = -1;
  u8 *buffer = NULL;
  u8 *cbuffer;
  u8 *ext;
  char *dot;

  file_open(fd, filename, read);

  if(!file_check_valid(fd))
    return -1;

  while(1)
  {
    file_read(fd, &data, sizeof(struct SZIPFileHeader));

    // zip file end
    if(data.Sig != 0x04034b50)
      break;

    if(data.FilenameLength >= (s32)sizeof(tmp))
      continue;

    file_read(fd, tmp, data.FilenameLength);
    tmp[data.FilenameLength] = 0; // end string

    if(data.ExtraFieldLength)
      file_seek(fd, data.ExtraFieldLength, SEEK_CUR);

    if(data.GeneralBitFlag & 0x0008)
    {
      file_read(fd, &data.DataDescriptor,
       sizeof(struct SZIPFileDataDescriptor));
    }

    dot = strrchr((char *)tmp, '.');
    if(dot == NULL || dot[1] == '\0')
      continue;

    ext = (u8 *)(dot + 1);

    // file is too big
    if(data.DataDescriptor.UncompressedSize > gamepak_ram_buffer_size)
      goto outcode;

    if(!strcasecmp((char *)ext, "bin") || !strcasecmp((char *)ext, "gba"))
    {
      buffer = gamepak_rom;

      // ok, found
      switch(data.CompressionMethod)
      {
        case 0:
          retval = data.DataDescriptor.UncompressedSize;
          file_read(fd, buffer, retval);

          goto outcode;

        case 8:
        {
          z_stream stream;
          int err;
          u32 expected_size = (u32)data.DataDescriptor.UncompressedSize;

          cbuffer = (u8 *)malloc(ZIP_BUFFER_SIZE);
          if(cbuffer == NULL)
            goto outcode;

          memset(&stream, 0, sizeof(stream));
          stream.next_out = (Bytef *)buffer;
          stream.avail_out = expected_size;
          stream.zalloc = (alloc_func)0;
          stream.zfree = (free_func)0;

          err = inflateInit2(&stream, -MAX_WBITS);
          if(err != Z_OK)
          {
            free(cbuffer);
            goto outcode;
          }

          file_read(fd, cbuffer, ZIP_BUFFER_SIZE);
          stream.next_in = (Bytef *)cbuffer;
          stream.avail_in = ZIP_BUFFER_SIZE;

          err = Z_OK;
          while(err != Z_STREAM_END)
          {
            err = inflate(&stream, Z_SYNC_FLUSH);
            if(err == Z_STREAM_END)
              break;

            if(err == Z_BUF_ERROR)
            {
              file_read(fd, cbuffer, ZIP_BUFFER_SIZE);
              stream.next_in = (Bytef *)cbuffer;
              stream.avail_in = ZIP_BUFFER_SIZE;
              err = Z_OK;
              continue;
            }

            if(err != Z_OK)
            {
              inflateEnd(&stream);
              free(cbuffer);
              goto outcode;
            }
          }

          inflateEnd(&stream);
          free(cbuffer);

          if(stream.total_out != expected_size)
            goto outcode;

          retval = (s32)expected_size;
          goto outcode;
        }
      }
    }
  }

outcode:
  file_close(fd);

  return retval;
}
