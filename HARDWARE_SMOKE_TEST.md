# Hardware Smoke Test — gPSPDC

Manual verification checklist for Dreamcast hardware or [Flycast](https://github.com/flyinghead/flycast). Run after building a disc with `dc/dc.sh` or loading `gdC.elf` in an emulator.

## Prerequisites

- `gba_bios.bin` at `/cd/gba_bios.bin` (16384 bytes, MD5 `a860e8c0b6d573d191e4ec7db1b1e4f6`)
- At least one valid `.gba` / `.bin` / `.zip` ROM in `/cd/gbaDC/`
- Serial output or emulator log window for `printf` messages (optional)

## Boot and fatal errors

| # | Test | Expected result |
|---|------|-----------------|
| 1 | Boot **without** `gba_bios.bin` on disc | On-screen message: path `/cd/gba_bios.bin`, size 16384, MD5; waits for Start; exits |
| 2 | Boot **with** BIOS but **no** ROMs in `/cd/gbaDC/` | ROM browser opens; shows "No game loaded yet." and version `gPSPDC 0.9.1-dc` |
| 3 | Attempt to load a **missing** ROM from CLI | On-screen "Could not load game ROM:" with filename; waits for Start |
| 4 | Select a **corrupt or invalid** ROM from the menu | Same on-screen load error as CLI (not a silent exit) |

## Core gameplay

| # | Test | Expected result |
|---|------|-----------------|
| 5 | Load a small homebrew or test ROM | Game renders at 240×160; audio plays; input responds |
| 6 | Open in-game menu, change frameskip, return | Settings apply; gameplay resumes |
| 7 | Save state to slot 0, load slot 0 | `<romname>.0.svs` created in `/cd/gbaDC/`; state restores correctly |
| 8 | Enable a known-good `.cht` cheat | Cheat takes effect in-game |

## Large ROM / paging

| # | Test | Expected result |
|---|------|-----------------|
| 9 | Load a ROM larger than the resident buffer (e.g. >8 MB) | Game loads via swap paging; no crash on title screen |

## Regression notes

Record any failures with: emulator or hardware revision, disc layout, ROM title, and serial log snippet. File issues against the `dreamcast` branch.
