# External Validation Candidates

This file tracks permissively licensed test packs, reference implementations,
and small dependencies that are useful for auditing gPSPDC without committing
third-party source or ROM binaries into this repository.

Use `sh scripts/fetch-external-validation.sh` to clone the test/reference
repos under `external/gba-validation/`. Set `DRY_RUN=1` to print the planned
fetches without cloning. The `external/` directory is intentionally ignored by
git. The fetched revisions used for the current integration pass are recorded
in `EXTERNAL_VALIDATION_LOCK.md`.

## Priority Plan

| Priority | Project | Fit | Integration |
|----------|---------|-----|-------------|
| P0 | `jsmolka/gba-tests` | Best single MIT GBA regression pack: ARM, Thumb, memory, BIOS, save/flash, PPU suites | Fetch locally, build ROMs with FASMARM, run manually in Flycast / hardware, record pass/fail in `scripts/smoke-test-results.md` |
| P0 | `SingleStepTests/ARM7TDMI` | Roughly 20k instruction-level JSON cases per category; ideal for interpreter and SH-4 dynarec parity | Add a future host harness that seeds gpSP CPU state, executes one instruction through interpreter and dynarec paths, and compares expected registers/memory |
| P1 | `miniz` | Vendored optional inflate backend for ZIP loading | Build with `USE_MINIZ=1` for heap-use experiments before replacing the default zlib path |
| P2 | `stb_easy_font` / `stb_image_resize` | Small UI helpers for clearer menu/error rendering and asset resizing | Consider only if current `font.h` and SDL blits become a real UI blocker |
| P3 | `SkyEmu`, `rustboyadvance-ng`, `gba-kit` | Read-only references for hard-game behavior, flash/SRAM/EEPROM, bus, DMA, and timers | Audit against specific bugs; do not port code unless the license notice is added and the adaptation is small |
| P4 | KOS / `mkdcdisc` pinning | Easier contributor builds and reproducible CDI packaging | Keep the Docker image pinned; document any local tool version that affects bootable disc output |

## Test ROM Packs

| Project | License | Link | Use for gPSPDC |
|---------|---------|------|----------------|
| `jsmolka/gba-tests` | MIT | <https://github.com/jsmolka/gba-tests> | ARM, Thumb, memory, BIOS, save/flash, and PPU suites |
| `SingleStepTests/ARM7TDMI` | MIT | <https://github.com/SingleStepTests/ARM7TDMI> | Instruction-level ARM7TDMI JSON tests for interpreter / dynarec parity |
| `destoer/gba_tests` | MIT | <https://github.com/destoer/gba_tests> | Extra timing and behavior ROMs, including midline/VCOUNT style cases |
| `ladystarbreeze/GBA-Test-Collection` | MIT | <https://github.com/ladystarbreeze/GBA-Test-Collection> | Mid-frame DMA and interrupt timing cases |

## Reference Implementations

| Project | License | Link | Relevant gaps |
|---------|---------|------|---------------|
| `michelhe/rustboyadvance-ng` | MIT | <https://github.com/michelhe/rustboyadvance-ng> | ARM7TDMI and cartridge backup logic for SRAM, flash, and EEPROM audits |
| `macabeus/gba-kit` | MIT | <https://github.com/macabeus/gba-kit> | Modular TypeScript core for bus, DMA, and timer behavior cross-checks |
| `skylersaleh/SkyEmu` | MIT | <https://github.com/skylersaleh/SkyEmu> | Read-only reference for hard-game behavior and cheat/save edge cases |

## Candidate Small Dependencies

| Project | License | Link | Notes |
|---------|---------|------|-------|
| `richgel999/miniz` | MIT | <https://github.com/richgel999/miniz> | Vendored under `third_party/miniz/` as an optional `zip.c` inflate backend |
| `nothings/stb` | Public domain / MIT-style dual license | <https://github.com/nothings/stb> | `stb_easy_font` and `stb_image_resize` are small optional UI/build helpers |

## Suggested Workflow

1. Run `sh scripts/fetch-external-validation.sh`.
2. Build only the ROM packs needed for the bug being investigated.
3. Copy selected `.gba` outputs to `/cd/gbaDC/` for Flycast or hardware tests.
4. Log results in `scripts/smoke-test-results.md` with the project, commit hash,
   ROM name, emulator mode, and observed result.
5. When adapting code, update `THIRD_PARTY_NOTICES.md` in the same commit.

Do not commit BIOS files, commercial ROMs, generated `.gba` test binaries, or
large JSON test corpora. Keep those under `external/` or another local path.
