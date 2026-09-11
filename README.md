# gPSPDC

**gpSP** for the Sega Dreamcast — a Game Boy Advance emulator based on [gameplaySP](https://github.com/exophase/gpsp) (gpSP) by Exophase.

This repository (`dreamcast` branch) is an updated port of the original Dreamcast work by Troy Davis (GPF), with an SH-4 dynamic recompiler, SDL video, and ongoing compatibility fixes.

## Features

- GBA CPU, graphics, sound, DMA, timers, and cartridge backup (SRAM / flash / EEPROM)
- SH-4 dynarec for improved emulation speed (stock-hardware FPS not yet measured)
- In-game menu: load games, savestates, frameskip, scaling, cheats, per-game config
- Zip ROM support (first `.gba` / `.bin` entry in the archive)
- GameShark / Pro Action Replay cheat files (`.cht`)
- Real-time clock (RTC) support for compatible cartridges
- Demand-paged ROM loading for large games (32 KB swap pages)

## Requirements

You must supply these files yourself; they are **not** included in the repository.

| File | Location on disc | Notes |
|------|------------------|-------|
| `gba_bios.bin` | `/cd/gba_bios.bin` | 16 KB (16384 bytes). MD5: `a860e8c0b6d573d191e4ec7db1b1e4f6` |
| GBA ROMs | `/cd/gbaDC/` | `.gba`, `.bin`, or `.zip` |
| `game_config.txt` | `/cd/gbaDC/` | Per-game tuning (included under `dc/cd/gbaDC/`) |

You are responsible for owning any BIOS or ROM images you use.

## Disc layout

When building a CD image (`dc/dc.sh`), the expected layout is:

```
/cd/
  gba_bios.bin          # GBA BIOS (required)
  gdC.elf               # Main binary (or 1ST_READ.BIN boot wrapper)
  gbaDC/
    game_config.txt     # Idle-loop and per-game settings
    *.gba / *.zip       # Your ROM files
    *.sav               # Optional pre-existing backup files
    *.svs               # Optional pre-existing savestates (slot 0-9)
    *.cht               # Cheat files (optional)
    gpsp.cfg            # Optional pre-existing global config
```

At startup the emulator changes directory to `/cd/gbaDC/` and loads the BIOS from `/cd/gba_bios.bin`.

## Controls

Default bindings follow the gpSP menu system. In general:

| Dreamcast | GBA |
|-----------|-----|
| D-Pad | D-Pad |
| A | A |
| B | B |
| Start | Start |
| X | B (default) |
| Y | Open pause menu (default; configurable) |
| L / R triggers | L / R |

**Menus:** D-Pad navigates; **A** or **Start** selects; **B** backs out or returns to game; **X** goes up one folder in the ROM browser. Open the in-game pause menu with **Y** by default (change under **Configure gamepad input**).

Frameskip is adjustable from **Graphics and Sound options** in the pause menu.

## Building

### Dreamcast (KOS)

Requirements:

- [KallistiOS](https://github.com/KallistiOS/KallistiOS) (KOS) with Dreamcast toolchain
- KOS ports: **SDL**, **zlib**, **libpng**
- Optional: pass `USE_MINIZ=1` to build ZIP inflate against vendored `miniz` instead of zlib for heap-use experiments
- Optional: `mkdcdisc`, `scramble` (for `dc/dc.sh` CD image creation)

```sh
cd dc
make
```

With Docker (same image as CI):

```sh
./scripts/dc-build.sh
```

This produces `gdC.elf`. To build a bootable CDI:

```sh
# Place gba_bios.bin in dc/cd/ first
./dc.sh
```

### Host SDL build (debugging)

```sh
make -C x86
# or from the repo root:
make
```

Requires host `gcc`, SDL 1.x development libraries, and `zlib`.

### Host tests

Requires a host C compiler, Make, and Python 3. The suite includes save/config fault-injection tests that compile the production functions with simulated I/O failures.

```sh
make -C tests test
```

## Large ROMs on stock Dreamcast

Commercial GBA titles can be up to 32 MB. gPSPDC loads them from GD-ROM under `/cd/gbaDC/` using **32 KB demand paging** when the ROM is larger than the resident buffer.

| Topic | Detail |
|-------|--------|
| Resident buffer | A conservative **4 MB** allocation at startup (`memory.c`), leaving heap headroom for KOS, SDL, and later allocations on a stock 16 MB Dreamcast. Anything larger is demand-paged. |
| Uncompressed ROMs | Use `.gba` or `.bin` for titles larger than the resident buffer. Paging reads from the open disc file during play. |
| Zip ROMs | The first `.gba`/`.bin` inside a `.zip` must fit **entirely** in the resident buffer. Larger zipped games will not load — extract to `.gba` on the disc instead. |
| Full speed | Paging from GD-ROM costs seek/read time on each 32 KB miss. Adjacent-page prefetch reduces sequential misses; titles still need `game_config.txt` idle-loop entries for dynarec speed. Use frameskip if needed. |
| Validation | See [HARDWARE_SMOKE_TEST.md](HARDWARE_SMOKE_TEST.md) test **#9** (large ROM title screen) before relying on a burned CDI. |

## Known limitations

- **VRAM self-modifying code** — executable code written to VRAM is not supported. Games that rely on this may misbehave; do not disable SMC checks globally.
- **Cheat hooks** — master-hook addresses are validated to ROM/EWRAM/IWRAM ranges, but malformed cheat codes can still corrupt game state or crash.
- **Hardware validation** — run [HARDWARE_SMOKE_TEST.md](HARDWARE_SMOKE_TEST.md) on Flycast or real hardware before release.

## Save storage limitations

Dreamcast starts in `/cd/gbaDC/`. A GD-ROM/CD image is read-only: ordinary file writes cannot store new `.sav`, `.svs`, or config files there. VMU persistence is not implemented. Existing files on the disc can be read, but **do not rely on a disc session to preserve new progress or settings**. Writable host storage is required for the current file-based save paths.

Failed saves show an on-screen message. If a save fails while exiting, acknowledge the message with A/Start before the application closes. Failed automatic backup writes remain pending in memory and retry after about five emulated seconds, with one notification per failure episode. Pending data is lost if you quit or replace the loaded game before a successful write.

## Configuration

- **`gpsp.cfg`** — global settings (video, audio buffer, controls). Read from `/cd/gbaDC/` on Dreamcast; changes cannot persist to a read-only disc.
- **`game_config.txt`** — per-title options (idle-loop targets, flash size, stack tweaks). See comments in the bundled file.
- **`<romname>.cfg`** — per-game frameskip and clock options, read next to the ROM; writing changes requires writable storage.

See [ROADMAP.md](ROADMAP.md) for the unified plan of remaining work, and [HIGH_IMPACT_FIXES.md](HIGH_IMPACT_FIXES.md) for the record of completed port work. Before burning a disc, run the checks in [HARDWARE_SMOKE_TEST.md](HARDWARE_SMOKE_TEST.md). External MIT test packs and reference projects are tracked in [EXTERNAL_VALIDATION.md](EXTERNAL_VALIDATION.md). Third-party code attributions are in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Cheats

Create a `.cht` file next to your ROM with the same basename. Example:

```
gameshark_v3 Infinite Health
995fa0d9 0c6720d2
```

Enable cheats from the in-game **Cheats/Misc** menu. Gameshark v1/v3 write codes, ROM patches, button-gated codes, master-hook addresses, IF/conditional codes, and PAR v3 conditionals are supported. Invalid hook PCs outside ROM/RAM are ignored; other bad codes may still crash a game.

## Savestates

Save and load from the menu or bound buttons. Savestate filenames are `<romname>.0.svs` … `<romname>.9.svs` in `/cd/gbaDC/`. They are not compatible with other emulators. Saving requires writable storage; a burned disc cannot retain new savestates. Failed saves show a nonfatal on-screen message.

## Credits

- **Exophase** — original gpSP / gameplaySP
- **Troy Davis (GPF)** — original Dreamcast port
- **SiberianSTAR** — zip support (upstream gpSP)
- Community contributors — SH-4 dynarec, fixes, and testing on this fork

## License

gameplaySP is free software, licensed under the **GNU General Public License v2.0**.

See [LICENSE](LICENSE) for the full text.

Portions of this tree retain the original gpSP file headers and copyright notices. Modifications and Dreamcast-specific code in this repository are offered under the same license.
