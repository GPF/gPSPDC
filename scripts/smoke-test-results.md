# Hardware Smoke Test Results

Record Flycast or Dreamcast results for [HARDWARE_SMOKE_TEST.md](../HARDWARE_SMOKE_TEST.md).
Follow the build + run procedure in [flycast-test.md](flycast-test.md).

**Host contract tests** (`make -C tests test`): pass on CI — these verify source contracts only, not on-target FPS, menu feel, or large-ROM gameplay.

| # | Test | Pass/Fail | Notes |
|---|------|-----------|-------|
| 1 | Boot without BIOS | | |
| 2 | Boot with BIOS, no ROMs | | |
| 3 | CLI missing ROM | | |
| 4 | Menu invalid ROM | | |
| 5 | Small ROM gameplay | | |
| 6 | Menu frameskip change | | |
| 7 | Savestate slot 0 | | |
| 8 | Known-good cheat | | |
| 9 | Large ROM paging | | |
| 10 | Menu navigation | | |
| 11 | Menu hold CPU use | | |
| 12 | Savestate thumbnail | | |
| 13 | ROM browser | | |
| 14 | ROM buffer fatal error | | |
| 15 | Dynarec fatal error | | |

**Environment:** Flycast win64-2.6 (known-good), CDI load. Known-good config:
`RamMod32MB=yes`, `Dynarec.Enabled=yes`, `Sh4Clock=200`, `UseReios=no`,
`FastGDRomLoad=no`, `pvr.rend=2`, `rend.ThreadedRendering=yes`,
`aica.BufferSize=2822`. See [flycast-test.md](flycast-test.md) §2.

**Disc:** `dc/gbapspDC.cdi` from `dc/dc.sh`; autoloads `DangerousXmas.bin`.

**Date:**

**Branch / commit:**
