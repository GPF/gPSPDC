# Third-Party Notices

gPSPDC incorporates ideas and adapted code from the following open-source projects.

## SkyEmu

- **Project:** [SkyEmu](https://github.com/skylersaleh/SkyEmu) by Skyler Saleh
- **License:** [MIT License](https://github.com/skylersaleh/SkyEmu/blob/dev/LICENSE)
- **Used in:** `cheats.c` — Action Replay / Gameshark v3 conditional cheat decoding (`gba_run_ar_cheat` / `gba_handle_ar_if_instruction` logic, adapted for gpSP's memory helpers and pre-decrypted cheat lines)

MIT License excerpt:

```
Copyright (c) 2021 Skyler "Sky" Saleh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
