# Flycast Test Runbook — gPSPDC

Step-by-step procedure for the Track A1 Flycast smoke test
([ROADMAP.md](../ROADMAP.md)). Goal: build a self-contained CDI from a clean
checkout and run the 15-row [HARDWARE_SMOKE_TEST.md](../HARDWARE_SMOKE_TEST.md)
checklist in [Flycast](https://github.com/flyinghead/flycast), recording results
in [smoke-test-results.md](smoke-test-results.md).

The repo already ships everything needed for a license-clean first boot:

| File | Purpose |
|------|---------|
| `dc/cd/gba_bios.bin` | GBA BIOS (MD5 `a860e8c0b6d573d191e4ec7db1b1e4f6`) |
| `dc/cd/gbaDC/DangerousXmas.bin` | Free homebrew GBA ROM (render/input/audio test) |
| `dc/cd/gbaDC/autoload.txt` | Set to `DangerousXmas.bin` — fresh CDI boots straight into it |
| `dc/cd/gbaDC/game_config.txt` | Per-game tuning |

To smoke-test commercial titles, drop a `.gba`/`.bin`/`.zip` into
`dc/cd/gbaDC/` and either edit `autoload.txt` to its filename or pick it from
the in-game ROM browser. `autoload.txt` is a local override — leave the
committed value at `DangerousXmas.bin` so a clean checkout always boots.

## 1. Build the binary and CDI

This container cannot cross-compile (no KOS toolchain / Docker daemon). Run the
build on a machine with Docker:

```sh
# From repo root — validate the disc layout first (BIOS, autoload ROM, config).
# dc.sh runs this automatically, but checking up front catches problems before
# the slow cross-compile.
./scripts/check-disc.sh

# Clean rebuild of the Dreamcast ELF via the CI toolchain image
rm -f dc/gdC.elf
./scripts/dc-build.sh            # produces dc/gdC.elf

# Package a bootable CDI (needs sh-elf-objcopy, scramble, mkdcdisc on PATH,
# e.g. from a KOS install or a disposable Docker container)
cd dc && ./dc.sh                 # produces dc/gbapspDC.cdi
```

`dc.sh` runs `scripts/sync-game-config.sh` first so the disc's
`game_config.txt` matches the repo root copy. Confirm the host contract suite is
green before building:

```sh
make -C tests test
```

## 2. Stock-memory validation configuration

Use these settings for the next stock 16 MB validation run. The June 12
report observed gameplay on Flycast `win64-2.6` with 32 MB enabled; it
does not establish a pass for this configuration.

```text
Dreamcast.RamMod32MB = no       # Stock 16 MB validation baseline
Dynarec.Enabled     = yes
Sh4Clock            = 200
UseReios            = no         # use the real flow, not HLE BIOS
FastGDRomLoad       = no
pvr.rend            = 2
rend.EmulateFramebuffer = no
rend.ThreadedRendering  = yes
aica.BufferSize     = 2822
```

Optional: enable `Debug.SerialConsoleEnabled` to capture the emulator's
`printf` boot/fatal messages.

## 3. Run the checklist

Load `dc/gbapspDC.cdi` in Flycast and work through
[HARDWARE_SMOKE_TEST.md](../HARDWARE_SMOKE_TEST.md). Expected behavior with the
shipped disc:

| Row | What to do | Expected |
|-----|-----------|----------|
| 1 | Build a CDI without `gba_bios.bin` | On-screen BIOS-missing message (path/size/MD5), waits for Start |
| 2 | Empty `gbaDC/` (and no autoload) | ROM browser shows "No game loaded yet." and `gPSPDC 0.9.1-dc` |
| 3/4 | Autoload/select a missing or corrupt ROM | On-screen "Could not load game ROM:" — not a silent exit |
| 5 | Default boot (autoloads `DangerousXmas.bin`) | Renders 240×160 in the upper-left, audio plays, input responds |
| 6 | Pause menu → change frameskip → resume | Setting applies, gameplay resumes |
| 7 | Save state slot 0, then load slot 0 | `DangerousXmas.0.svs` created in `/cd/gbaDC/`, state restores |
| 8 | Enable a known-good `.cht` | Cheat takes effect |
| 9 | Swap in a >8 MB ROM, boot it | Loads via 32 KB demand paging, no crash on title |
| 10–13 | Navigate menus / ROM browser | Immediate highlight, low idle CPU, responsive listing |
| 14 | Simulate ROM-buffer alloc failure | On-screen "could not allocate ROM buffer", waits for Start |
| 15 | Trigger a dynarec translation failure | On-screen "Dynarec translation failed:" with PC, waits for Start |

Note: native GBA 240×160 drawn in the upper-left of the Dreamcast surface is the
documented framebuffer behavior, not a boot failure.

## 4. Record results

Fill in pass/fail for every row in
[smoke-test-results.md](smoke-test-results.md), including the Flycast version and
the date. Any failure is a concrete repro to feed back into Track B/C of the
roadmap. When rows 1–8 pass, Track E1 (tag a release) is unblocked.

## Validation baseline

Use `Dreamcast.RamMod32MB = no` for stock Dreamcast checks, including large-ROM paging. This is the required baseline, not a recorded pass. The June 12 gameplay observations used `RamMod32MB = yes`; keep those as separate 32 MB observations. Record the tested commit, any local changes, CDI SHA-256, Flycast version, RAM setting, and each observed result in `smoke-test-results.md`.
