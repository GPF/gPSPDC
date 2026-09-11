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

**Historical environment:** Flycast win64-2.6, CDI load. The June 12 reports record gameplay with the following 32 MB configuration; this is not a completed stock-hardware checklist:
`RamMod32MB=yes`, `Dynarec.Enabled=yes`, `Sh4Clock=200`, `UseReios=no`,
`FastGDRomLoad=no`, `pvr.rend=2`, `rend.ThreadedRendering=yes`,
`aica.BufferSize=2822`. See [flycast-test.md](flycast-test.md) §2.

**Disc:** `dc/gbapspDC.cdi` from `dc/dc.sh`; autoloads `DangerousXmas.bin`.

**Date:**

**Branch / commit:**

## Next stock-memory run (not yet performed)

- Flycast version:
- Commit and local changes:
- CDI SHA-256:
- RAM setting: `RamMod32MB=no` (16 MB)
- Date:
- Results: fill the table only for tests actually performed.

The June 12 debug report records Tekken Advance gameplay on an older checkout with local changes and 32 MB enabled. Do not transfer that observation to the empty checklist or label it a 16 MB pass.

---

## 2026-09-10 — first frame-rate measurement (Flycast 2.7, stock 16 MB)

The first speed numbers this port has produced. Until now no frame rate had
ever been recorded, because the Dreamcast build reports timing through
`SDL_WM_SetCaption`, which does nothing on target.

**How it was read.** The Dreamcast build presents exactly one flip per emulated
GBA frame (`update_screen` calls `flip_screen` whenever `skip_next_frame` is
clear, and on this path it never sets). So Flycast's own frame counter reports
the emulated GBA frame rate directly. Enabled with `rend.ShowFPS = yes`; no
other Flycast setting was changed, so RAM is stock 16 MB and the SH-4 clock is
the default 200 MHz.

| Workload | Frames per second | Share of 59.73 target |
|---|---|---|
| `DangerousXmas.bin` title screen (1.3 MB homebrew, fully resident) | 53.9 | 90% |
| Super Puzzle Fighter II, character intro | 29.0 | 49% |
| Super Puzzle Fighter II, attract-mode match, 10 samples over 65 s | mean 23.2, median 26.6 | 39% |
| Super Puzzle Fighter II, worst sample | 8.5 | 14% |
| Super Puzzle Fighter II, best sample | 32.0 | 54% |

Samples: 28.9, 27.2, 25.2, 8.5, 19.6, 9.0, 26.1, 27.9, 28.0, 32.0. A separate
reading during a two-player match showed 23.1, and one during a heavy
gem-clearing sequence showed 5.8.

The homebrew control matters as much as the game numbers: it shows the
disc, video and flip path can sustain roughly 54 fps, so the deficit on
commercial code is recompiler throughput, not presentation.

**Caveat.** Flycast's SH-4 timing is approximate rather than cycle-exact, so
these are first-order estimates of Dreamcast speed, not hardware measurements.
Real GD-ROM seek latency is also not reproduced, so large-ROM paging cost does
not appear here at all.

### Games tested

| ROM | Size | Paging | Result |
|---|---|---|---|
| `DangerousXmas.bin` | 1.3 MB | none, fully resident | Boots and runs |
| Super Puzzle Fighter II (USA) (Rev 1) | 8 MB | demand-paged | Boots, attract mode stable past 65 s |
| Grand Theft Auto Advance (USA) | 16 MB | demand-paged | Boots to title, then **dynarec fatal error** |

### Open defect

Grand Theft Auto Advance reaches its title sequence and then stops with:

```
Dynarec translation failed:
bad jump 1a3019f8 (3000154) (0)
```

The requested block address `0x1a3019f8` is not a valid GBA region, so
`block_lookup_address_*` reaches its `default:` case. The CPU was executing in
IWRAM at `0x03000154`. Two smaller titles, one of them also demand-paged, do
not reproduce it, so ROM paging alone is not a sufficient explanation.
Reproduced on the first two runs; not yet isolated.

**Disc images used:** built with `mkisofs -C 0,11702` plus `cdi4dc`, with a
`scramble`d `1ST_READ.BIN`. An unscrambled binary boots as far as the Sega
licence screen and then hangs, so scrambling is required for this bootstrap.
