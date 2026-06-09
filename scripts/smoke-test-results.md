# Hardware Smoke Test Results

Record Flycast or Dreamcast results for [HARDWARE_SMOKE_TEST.md](../HARDWARE_SMOKE_TEST.md).

**Host contract tests** (`make -C tests test`): pass on CI — these verify source contracts only, not on-target FPS or large-ROM gameplay.

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

**Environment:** (Flycast version / DC hardware / disc or ELF load method)

**Date:**

**Branch / commit:**
