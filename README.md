# gPSPDC

**gpSP** for the Sega Dreamcast — a Game Boy Advance emulator based on [gameplaySP](https://github.com/exophase/gpsp) (gpSP) by Exophase.

This repository (`dreamcast` branch) is an updated port of the original Dreamcast work by Troy Davis (GPF), with an SH-4 dynamic recompiler, SDL video, and ongoing compatibility fixes.

## Features

- GBA CPU, graphics, sound, DMA, timers, and cartridge backup (SRAM / flash / EEPROM)
- SH-4 dynarec for full-speed emulation on real hardware
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
    *.sav               # Save files (created at runtime)
    *.svs               # Savestates (slot 0-9)
    *.cht               # Cheat files (optional)
    gpsp.cfg            # Global config (created at runtime)
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
| X / Y | Select / menu shortcuts (configurable) |
| L / R triggers | L / R |

Open the menu from the ROM browser or in-game (default: hold the menu trigger and use the D-Pad). Button mappings can be changed in **Options → Controls**.

Frameskip is adjustable from the menu or the in-game frameskip bar.

## Building

### Dreamcast (KOS)

Requirements:

- [KallistiOS](https://github.com/KallistiOS/KallistiOS) (KOS) with Dreamcast toolchain
- KOS ports: **SDL**, **zlib**, **libpng**
- Optional: `mkdcdisc`, `scramble` (for `dc/dc.sh` CD image creation)

```sh
cd dc
make
```

This produces `gdC.elf`. To build a bootable CDI:

```sh
# Place gba_bios.bin in dc/cd/ first
./dc.sh
```

### Host tests

```sh
make -C tests test
```

## Configuration

- **`gpsp.cfg`** — global settings (video, audio buffer, controls). Stored on the disc under `/cd/gbaDC/`.
- **`game_config.txt`** — per-title options (idle-loop targets, flash size, stack tweaks). See comments in the bundled file.
- **`<romname>.cfg`** — per-game frameskip and clock options, saved next to the ROM.

See [HIGH_IMPACT_FIXES.md](HIGH_IMPACT_FIXES.md) for recent port work and the development roadmap. Third-party code attributions are in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Cheats

Create a `.cht` file next to your ROM with the same basename. Example:

```
gameshark_v3 Infinite Health
995fa0d9 0c6720d2
```

Enable cheats from the in-game **Cheats/Misc** menu. Gameshark v1/v3 write codes, ROM patches, button-gated codes, master-hook addresses, IF/conditional codes, and PAR v3 conditionals are supported. Bad codes may still crash a game.

## Savestates

Save and load from the menu or bound buttons. Savestates are stored as `<romname>.0.svs` … `<romname>.9.svs` in `/cd/gbaDC/`. They are not compatible with other emulators.

## Credits

- **Exophase** — original gpSP / gameplaySP
- **Troy Davis (GPF)** — original Dreamcast port
- **SiberianSTAR** — zip support (upstream gpSP)
- Community contributors — SH-4 dynarec, fixes, and testing on this fork

## License

gameplaySP is free software, licensed under the **GNU General Public License v2.0**.

See [LICENSE](LICENSE) for the full text.

Portions of this tree retain the original gpSP file headers and copyright notices. Modifications and Dreamcast-specific code in this repository are offered under the same license.
