# High Impact Fixes — gPSP Dreamcast Port

Analysis of the highest-impact fixes for the **gPSPDC** Dreamcast port (`dreamcast` branch).

---

## Phase 0 — Restore `blit_to_screen()` (P0)

**Status:** Complete

`blit_to_screen()` in `video.c` was emptied when the SQ-optimized implementation was commented out. `gui.c` still calls it for savestate snapshot thumbnails (`blit_to_screen(snapshot_buffer, 240, 160, 230, 40)`).

- Main gameplay rendering via `SDL_Flip()` is unaffected.
- Menu overlays and savestate previews are broken without this function.

**Fix:** Restore the Dreamcast SQ blit path (from commit `a13177f`), with a simple memcpy fallback for non-Dreamcast builds.

---

## Phase 1 — Complete SH-4 dynarec (P1)

**Status:** Complete (audited and polished)

Dreamcast uses `execute_arm_translate()` via the SH-4 dynarec backend.

| Area | Status |
|------|--------|
| Load/store helpers | In `dc/sh4_helpers.c` and `dc/sh4_stub.c` (SMC-aware stores) |
| CPSR/SPSR/SWI | In `dc/sh4_helpers.c` |
| Conditional branches + idle loops | In `dc/sh4_emit.h` |
| Block memory (dynarec) | Full macros via `dc/sh4_instr.inc` (x86 port) |
| `generate_update_pc_reg` | Passes `pc`, reloads cycle counter from `sh4_update_gba` return |

**Audit polish (Phase 1):**

- Extended `SH4_EMIT_LOAD/STORE_REG` for `reg[16+]` (flags, CPSR) via indexed addressing
- Fixed conditional branch patch (`generate_branch_patch_conditional`) and relative offset (`+4` delay slot)
- `sh4_update_gba` tail-jumps on IRQ/PC change; emitted code reloads `r13` from return value
- `execute_store_*` takes instruction PC (third arg) instead of corrupting `REG_PC` with the address
- Added `get_shift_imm` / `generate_shift_reg`; Thumb hi-reg PC branches move target into `r4`
- Audited SH-4 opcode emission for ALU ops, load/store displacement, shifts/rotates, `jsr`, `cmp/eq`, and unsigned immediate materialization
- Async exits from `sh4_update_gba` and store-alert helpers reload the live SH-4 `r13` cycle budget before block re-entry
- Post-compile icache invalidation now covers the active RAM/ROM/BIOS translation cache instead of always flushing the RAM cache span
- `SH4_EMIT_CMP_REG` uses `cmp/eq` (sets T flag); `swi_hle_div` remainder fix
- Host tests: `tests/phase1_helpers_test.c`, `tests/sh4_emit_encoding_test.c`, `tests/sh4_integration_contract_test.c`

**Remaining risks:** hardware validation on real Dreamcast/KOS toolchain; dynarec edge cases may still need game-specific testing.

---

## Phase 2 — CPU core LDM/STM fixes (P2)

**Status:** Complete

Fixed ARM `LDM`/`STM` in the interpreter (`cpu.c`):

1. **Writeback timing** — base register writeback now runs *after* the transfer loop, so `STM` with the base in the register list stores the original value before applying the final address.
2. **Load writeback** — when the base is in the list, the loaded value is kept (writeback skipped), matching ARM7TDMI behavior.
3. **User bank (`^` suffix)** — `LDM`/`STM` with the `s_bit` flag now access `reg_mode[MODE_USER][8–14]` for R8–R14 when in a privileged CPU mode.

Affects games using privileged-mode register bank switching or `STM`/`LDM` with writeback when the base register is included in the transfer list.

---

## Phase 3 — Memory limits on Dreamcast (P3)

**Status:** Complete

Unified gamepak swap paging in `memory.c`:

- **32 KB pages everywhere** — `GAMEPAK_SWAP_PAGE_SIZE` used for buffer page count, `load_gamepak_page()`, and `evict_gamepak_page()` (fixes prior 8 KB / 32 KB mismatch on Dreamcast).
- **Dreamcast ROM buffer** — tries **16 MB**, then **12 / 8 / 4 MB** on `malloc` failure (was a fixed 8 MB with wrong page math).
- **Graceful failure** — `init_gamepak_buffer()` returns early if allocation fails instead of dereferencing NULL.

**Impact:** Larger ROMs can stay resident longer on DC; swapped ROM paging is consistent across platforms.

---

## Phase 4 — Release polish (P4)

**Status:** Complete

- **Debug output** — startup and DMA trace `printf` calls in `main.c`, `memory.c`, and `video.c` now route through `gpsp_debug_printf()` (silent unless built with `-DGPSP_DEBUG`). User-facing errors (missing BIOS, failed ROM load) are unchanged.
- **Cheats** — fixed Gameshark v3 I/O register opcode extraction (`(address >> 24) & 0x0F`); added ROM patch (opcode `0x6`), button-gated writes (`0x8`), hook/master address tracking (`0x0F` and `0x001DC0DE` lines). Gameshark v1 IF codes (`0xD`/`0xE`) and PAR v3 conditionals are supported via an Action Replay engine adapted from [SkyEmu](https://github.com/skylersaleh/SkyEmu) (MIT): nested IF/ELSE/ENDIF stacks, ROM patches, fill codes, and button-gated multi-line writes. PAR v3 slowdown (`00000000 0800xx00`) re-runs the cheat list `xx` times per cycle. `DEADFACE` re-encryption lines reseed decryption during cheat load (mGBA algorithm/tables, MPL 2.0). Up to eight master hooks are tracked; dynarec emits `process_cheats()` at any hook PC via `cheat_pc_is_hook()` and flushes translation caches when hooks change. See `THIRD_PARTY_NOTICES.md`.
- **CPSR store** — `execute_store_cpsr()` in `dc/sh4_helpers.c` returns the IRQ vector when enabling interrupts unmasks a pending IRQ; dynarec emission branches via `arm_psr_store_cpsr_post()`. `execute_spsr_restore()` uses the same `sh4_take_pending_irq()` helper so MOVS PC returns through the vector instead of leaving IRQ state half-applied.
- **Video scaling** — left at native 240×160 on Dreamcast (`video_scale = 1`); integer scaling in `flip_screen()` remains disabled because the DC SDL path uses textured mode at GBA resolution. Menu scaling options affect window placement, not framebuffer upscale.
- **Host tests** — `tests/phase4_cheats_test.c` covers GS3 opcode extraction and master-hook address math.

---

## Phase 5 — User readiness audit (P5)

**Status:** Complete

- **On-screen fatal errors (Dreamcast)** — missing BIOS, ROM buffer allocation failure, and gamepak load errors now show a readable message on the display (with serial `printf` fallback) and wait for Start before exiting. BIOS screen includes `/cd/gba_bios.bin` path, size, and MD5.
- **ROM load error fix** — command-line load path now reports `argv[1]` instead of an uninitialized `load_filename`.
- **README** — savestate filenames corrected to `<romname>.0.svs` … `<romname>.9.svs` (matches `gui.c`).
- **Debug keys** — host SDL F2 palette dump gated behind `GPSP_DEBUG` (silent in release builds).
- **Host tests** — `tests/phase5_user_readiness_test.c` contract-checks fatal-error strings and README savestate docs.

**Remaining risks:** real-hardware smoke test on Dreamcast; no automated on-target UI test.

---

## Phase 6 — Release verification (P6)

**Status:** Complete

- **Version string** — `GPSPDC_VERSION` (`0.9.1-dc`) in `common.h`; shown on the empty-ROM menu splash.
- **Menu load errors (Dreamcast)** — failed in-menu ROM loads now call `gpsp_gamepak_load_error()` instead of silently exiting via `quit()`.
- **Hardware smoke test** — `HARDWARE_SMOKE_TEST.md` documents boot, fatal-error, gameplay, savestate, cheat, and large-ROM checks for Dreamcast/Flycast.
- **CI** — GitHub Actions workflow runs `make -C tests test` on push/PR to `dreamcast`.
- **Host tests** — `tests/phase6_release_verification_test.c` contract-checks version, menu error path, smoke-test doc, and CI workflow.

**Remaining risks:** manual hardware/Flycast execution of the smoke-test checklist.

---

## Phase 7 — Stable build (P7)

**Status:** Complete

- **KOS cross-compile CI** — `.github/workflows/dreamcast-build.yml` cross-compiles `gdC.elf` in `einsteinx2/dcdev-kos-toolchain:gcc-9` on push/PR to `dreamcast`.
- **Romdisk placeholder** — `dc/romdisk/` ensures KOS `genromfs` has a source directory in fresh checkouts.
- **Docker build helper** — `scripts/dc-build.sh` wraps the same container image for local stable builds.
- **Host tests** — `tests/dc_build_contract_test.c` contract-checks Dreamcast source paths, romdisk layout, and CI workflow.

**Remaining risks:** manual hardware/Flycast execution of the smoke-test checklist; CDI packaging (`dc/dc.sh`) still requires local KOS tools and user-supplied BIOS.

---

## Priority summary

| Phase | Fix | Effort | Impact |
|-------|-----|--------|--------|
| **0** | Restore `blit_to_screen` | Small | Fixes visible menu/savestate UI |
| **1** | Complete SH-4 dynarec stub + emit | Large | Full-speed play; idle-loop games |
| **2** | LDM/STM interpreter fixes | Medium | Compatibility for edge-case games ✓ |
| **3** | ROM buffer / paging strategy | Medium | Large ROM support ✓ |
| **4** | Release polish | Small | Cleaner release build ✓ |
| **5** | User readiness audit | Small | On-screen errors, docs, debug gating ✓ |
| **6** | Release verification | Small | Version, smoke-test doc, CI, menu errors ✓ |
| **7** | Stable build | Small | KOS cross-compile CI, romdisk, docker helper ✓ |

```mermaid
flowchart TD
    A[Phase 0: Restore blit_to_screen] --> B[Phase 1: Complete SH4 stub helpers]
    B --> C[Port conditional branches + idle loops]
    C --> D[Enable execute_arm_translate on DC]
    E[Phase 2: Fix LDM/STM interpreter bugs] --> D
    F[Phase 3: Evaluate ROM buffer sizing] --> G[Phase 4: Release polish]
    D --> G
    G --> H[Phase 5: User readiness audit]
    H --> I[Phase 6: Release verification]
    I --> J[Phase 7: Stable build CI]
```

---

## Already addressed (recent commits)

- Cheat code loading from `/cd/gbaDC/`
- Config filename path for Dreamcast
- Double buffering for video mode
- Menu resolution fix
