# gPSPDC Debug Report - 2026-06-12

## Summary

- Status: Flycast is running and responsive.
- Runtime image: `dc/gbapspDC-fresh.cdi`
- Autoload ROM: `Tekken Advance (USA).gba`
- Observed state: Tekken Advance boots from the CDI, reaches title, accepts input, and reaches in-game rendering.
- Live screenshot: `C:\Users\allen\AppData\Local\Temp\codex-shot-2026-06-12_22-35-49.png`

## Repository State

- Branch: `dreamcast`
- HEAD: `6a589af38906518adc920cf689fe90676478cf92`
- HEAD summary: `6a589af (HEAD -> dreamcast, origin/dreamcast, origin/HEAD) Merge pull request #25 from awest813/claude/dynarec-audit-roadmap-hed517`
- Local tracked changes:
  - `cheats.h`
  - `dc/sh4_stub.c`
  - `main.c`
  - `tests/sh4_integration_contract_test.c`
- Local runtime/test artifacts:
  - `dc/cd/gbaDC/autoload.txt`
  - `dc/gbapspDC-fresh.cdi`
  - previous trace CDI/screenshot/log artifacts remain untracked

## Source Changes In This Debug Pass

- `tests/sh4_integration_contract_test.c`
  - Normalizes CRLF to LF in loaded text fixtures so multiline contract checks do not false-fail on Windows checkouts.
- `cheats.h`
  - `MAX_CHEATS` increased from 8 to 10 to match the 10 cheat menu slots in `gui.c`.
- `dc/sh4_stub.c`
  - Runtime trace counter is compiled only when `GPSP_DC_RUNTIME_TRACE` is enabled.
- `main.c`
  - Non-trace `GPSP_DC_TRACE_EVERY` macro now marks its counter as used.
  - Removed unused locals from `main()`.

## Live Flycast State

- Process: `flycast.exe`
- Path: `C:\Users\allen\Downloads\flycast-win64-2.6\flycast.exe`
- Window title: `Flycast - gbapspDC-fresh`
- Responding: yes
- Window capture region: `344,130,800,600`
- Last observed gameplay: Tekken Advance in-game match with visible HUD/sprites.

Relevant Flycast config:

```text
Debug.SerialConsoleEnabled = no
Debug.SerialPTY = no
Dreamcast.RamMod32MB = yes
Dynarec.Enabled = yes
FastGDRomLoad = no
Sh4Clock = 200
UseReios = no
aica.BufferSize = 2822
pvr.rend = 2
rend.EmulateFramebuffer = no
rend.Resolution = 480
rend.ThreadedRendering = yes
```

## Disc Layout And Artifacts

```text
dc/gbapspDC-fresh.cdi                         63,988,681 bytes
dc/gdC.elf                                    10,215,344 bytes
dc/cd/1ST_READ.BIN                             1,659,536 bytes
dc/cd/gba_bios.bin                                16,384 bytes
dc/cd/gbaDC/autoload.txt                              25 bytes
dc/cd/gbaDC/Tekken Advance (USA).gba           8,388,608 bytes
```

Autoload contents:

```text
Tekken Advance (USA).gba
```

Hashes:

```text
SHA256 dc/gbapspDC-fresh.cdi
6A8BDDC69920A59C8DC4487DB39AF4AAD583FF966CA59889AD0F043B00ACC04F

SHA256 dc/gdC.elf
448743074D1F873C52AADEF04869591C3208DBA37DCC802276A5AF14120C3F63

SHA256 dc/cd/1ST_READ.BIN
25D879FAB51519BF51A804A53415F23B974A2B44EFEA98606D3940C02725A162

MD5 dc/cd/gba_bios.bin
A860E8C0B6D573D191E4EC7DB1B1E4F6

SHA256 dc/cd/gbaDC/Tekken Advance (USA).gba
09FC5EC1649EC162DA49044D6ECC84D5C74938EE315B902642BFDA0728A95333
```

## Build And Test Results

Host contract suite:

```text
make -C tests test
Result: pass
```

Dreamcast build check:

```text
./scripts/dc-build.sh all
Result: pass/current
Output: make: Nothing to be done for 'all'.
```

Clean KOS rebuild was also completed earlier in this pass before the final CDI was generated.

CDI packaging:

```text
mkdcdisc built in disposable Ubuntu 24.04 Docker container
Output CDI: dc/gbapspDC-fresh.cdi
Result: pass
```

## ROM Inventory

ROM source folder: `C:\Users\allen\Downloads\Roms`

```text
Legend of the River King GB (USA) (SGB Enhanced).zip                  195,611
Legend of Zelda, The - Link's Awakening (USA, Europe) (Rev 2).zip     296,379
Bomberman Max - Blue Champion (USA).zip                               429,123
Donkey Kong Country (USA, Europe) (En,Fr,De,Es,It).zip             1,233,052
Dragon Warrior III (USA).zip                                       2,006,425
Kirby & the Amazing Mirror.zip                                     5,825,015
Tekken Advance (USA).gba                                           8,388,608
Mario Tennis - Power Tour (USA, Australia) (En,Fr,De,Es,It).gba    16,777,216
F-Zero - GP Legend (USA).gba                                      16,777,216
Fire Emblem Fates - Conquest (USA).zip                         1,529,588,038
```

Notes:

- `Tekken Advance (USA).gba` was selected for this debug CDI because it is a direct GBA ROM and small enough for quick validation.
- Several listed files are Game Boy/Game Boy Color/SNES/3DS or otherwise not useful for this GBA emulator test pass.

## Runtime Observations

- Direct ELF loading in Flycast works for showing fatal screens, but it does not mount the app's expected `/cd` filesystem path, so it cannot find `/cd/gba_bios.bin`.
- A real CDI is required for meaningful ROM testing.
- The fresh CDI boots through the Dreamcast image path, loads BIOS and ROM from `/cd`, and autoloads Tekken.
- Current video behavior is native GBA 240x160 drawn in the upper-left of a larger Dreamcast/Flycast surface. This matches the current documented native-framebuffer behavior and is not a boot failure.
- Flycast remained responsive after reaching gameplay.

## Remaining Risks / Follow-Up

- Only Tekken Advance was runtime-tested in this report.
- Audio was not objectively captured or measured in this pass.
- Large ROM paging was not retested with `F-Zero - GP Legend` or `Mario Tennis - Power Tour`.
- Direct local `dc/dc.sh` still depends on `mkdcdisc` being installed on PATH; this report used a disposable Docker-built `mkdcdisc`.
- Dreamcast build still emits broad legacy warning noise from shared headers and dynarec macro expansion; the targeted warnings from this pass were reduced.
