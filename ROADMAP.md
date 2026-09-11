# gPSPDC Roadmap — Unified Plan

Single source of truth for everything still open on the Dreamcast port.
This consolidates the "remaining risks" scattered through
[HIGH_IMPACT_FIXES.md](HIGH_IMPACT_FIXES.md) (phases 0–11),
[EXTERNAL_VALIDATION.md](EXTERNAL_VALIDATION.md),
[HARDWARE_SMOKE_TEST.md](HARDWARE_SMOKE_TEST.md), in-code FIXMEs, and the
dynarec audit passes. Completed work stays documented in
HIGH_IMPACT_FIXES.md; this file only tracks what is **not done yet**.

Status keys: `open` (nobody started), `blocked` (needs hardware/emulator or a
user decision), `parity` (matches upstream gpSP behavior — fix only with a
test case proving it matters).

---

## Track A — Hardware / emulator validation (highest value, blocked on a tester)

The June 12 debug/performance reports record Tekken Advance gameplay in
Flycast on an older checkout with local changes and 32 MB RAM enabled.
The full checklist and stock 16 MB validation remain unperformed. Current
checkout correctness and real-hardware performance are still open risks.

| # | Item | Status | Notes |
|---|------|--------|-------|
| A1 | Run the 15-row checklist in [HARDWARE_SMOKE_TEST.md](HARDWARE_SMOKE_TEST.md) on Flycast | open | Record results in `scripts/smoke-test-results.md` (template ready, all rows empty) |
| A2 | Repeat the checklist on real Dreamcast hardware (burned CDI via `dc/dc.sh`) | blocked | Needs hardware + burner; serial cable optional for `printf` |
| A3 | Gameplay soak test: one ARM-heavy and one Thumb-heavy commercial game, 30+ min each | open | Watch for the classes of bug fixed in audits 1–3: IRQ timing, flag corruption after DMA IRQs, SMLAL/UMLAL audio voices, savestates after IRQs |
| A4 | Audio buffer tuning on hardware (`audio_buffer_size_number` sweep) | blocked | Phase 8 risk note; needs ears on hardware |
| A5 | Menu feel / input latency on hardware | blocked | Phase 8 risk note |
| A6 | Idle-loop profile: confirm `idle_loop_target_pc` entries in `game_config.txt` still fire under the SH-4 dynarec | open | The five `game_config.txt` "TODO: idle loop" comments are per-game gaps, not code bugs |

## Track B — Dynarec correctness (remaining, ranked)

Audit passes 1–3 fixed the known broken cases (see HIGH_IMPACT_FIXES.md
Phase 11 for audit 3). What's left is either upstream-parity behavior or
needs a failing test to justify divergence.

| # | Item | Status | Notes |
|---|------|--------|-------|
| B1 | `LDM rlist^` with PC in list does not restore CPSR from SPSR | parity | Interpreter and x86/MIPS backends share the gap; ARM7TDMI restores SPSR here. Needs a `jsmolka/gba-tests` ARM-suite repro before fixing all cores together |
| B2 | Indirect branches do not flush the pending translate-time `cycle_count` | parity | x86 reference has the same flush commented out upstream; cycles are under-billed on register-target branches. Fix in both backends at once or not at all (timing shifts can re-break games tuned around it) |
| B3 | Thumb `BX PC` uses `pc + 4` without word alignment | parity | Hardware uses `Align(pc,4) + 4`; only differs when the BX sits at a non-word-aligned Thumb address. Same in x86 backend |
| B4 | IRQ entry zeroes NZCV in the interpreter but preserves them in the dynarec flag registers | parity | Real hardware preserves NZCV (dynarec behavior is the accurate one); unify when the SingleStepTests harness (C2) exists |
| B5 | `execute_store_cpsr` IRQ return re-executes the MSR (LR = pc+4 instead of pc+8) | parity | Harmless for idempotent MSR; matches x86 in-tree reference. Revisit with single-step tests |
| B6 | SWI does not set the I bit (IRQ disable) on entry | parity | All gpSP cores share this; BIOS handler runs with IRQs at prior state for a few instructions |
| B7 | `SH4_ARM_MAX_EMIT_BYTES_PER_INSN` (512) is an estimate; the patch-time backstop calls `gpsp_dynarec_fatal_error` if it is ever wrong | open | If the fatal error is ever observed, measure the real worst-case emission and raise the bound (or always use the far skip) |
| B8 | `SH4_EMIT_LOAD_IMM` materializes 32-bit constants in up to 14 instructions | open | Perf, not correctness: a PC-relative literal pool would shrink hot blocks substantially (every helper call embeds a function address). Largest remaining dynarec speed lever |
| B9 | Block-level register allocation (gpSP "memory form" only on SH-4) | open | Every ARM register access is a load/store through r12. Big perf project; only attempt after A-track baselining shows it is needed |

## Track C — Test infrastructure

| # | Item | Status | Notes |
|---|------|--------|-------|
| C1 | Build `jsmolka/gba-tests` ROMs and run ARM/Thumb/memory/BIOS suites in Flycast; log in `scripts/smoke-test-results.md` | open | Repos already pinned in `EXTERNAL_VALIDATION_LOCK.md`; needs FASMARM locally |
| C2 | Host single-step harness around `SingleStepTests/ARM7TDMI` JSON: seed CPU state, step interpreter, compare | open | Phase 10 P0. Interpreter first (host-buildable); SH-4 dynarec comparison needs an SH-4 emulator or on-target runner — keep that part aspirational |
| C3 | Extend the emitted-code simulator beyond immediate loads and conditional skips | open | LOAD_IMM round trips and near/literal-veneer skip polarity now execute in host tests; expand toward memory and helper-call sequences |
| C4 | Extend host-compiled SH-4 helper tests to LDM/STM and real memory/IRQ side effects | open | Arithmetic, register shifts, and masked CPSR writes now have executable coverage; see [September 10 audit](DYNAREC_AUDIT_2026-09-10.md) |
| C5 | CI: cache the KOS Docker image pull (currently re-pulled every run) | open | `einsteinx2/dcdev-kos-toolchain:gcc-9__v2.0.0` |

## Track D — Platform / app polish

| # | Item | Status | Notes |
|---|------|--------|-------|
| D1 | `input.c:902` — "FIXME: Not implemented properly for x86 version" (host SDL input path) | open | Affects host debugging builds only |
| D2 | Host x86 dynarec build is untested in CI (root `Makefile` → `x86/`) | open | Audit 4 restored C-level compilation (`translation_ptr_t`, 4-arg `arm_block_memory` via `execute_arm_block_memory` helper, `generate_update_pc_reg`); remaining gap is `x86_stub.S` + link, which need a 32-bit toolchain (`as --32`, `-m32`, 32-bit SDL) |
| D3 | `USE_MINIZ=1` ZIP backend: build and compare heap headroom vs zlib on DC | open | Phase 10 P1; miniz already vendored |
| D4 | Persistent Dreamcast saves / VMU support | open | Disc paths are read-only. Failure reporting and bounded backup retries are implemented; writable storage support, VMU packaging/capacity, and recovery remain required |
| D5 | BIOS-free boot (HLE BIOS) | open | Large; upstream gpSP never fully solved it. Keep documented as out of scope unless demand appears |
| D6 | `stb_easy_font` / `stb_image_resize` for menu readability | open | Phase 10 P2 — only if the current font becomes a real blocker |

## Track E — Release engineering

| # | Item | Status | Notes |
|---|------|--------|-------|
| E1 | Tag a release once Track A rows 1–8 pass on Flycast (`GPSPDC_VERSION` bump + CDI artifact) | blocked on A | Version string lives in `common.h` |
| E2 | Publish per-release `scripts/smoke-test-results.md` snapshot | blocked on A | |
| E3 | Document a known-good Flycast version for contributors | open | One line in README once A1 runs |

---

## Suggested order of attack

1. **A1 (Flycast smoke test)** — everything else is hypothesis until the
   image boots and plays. Any failure feeds Track B/C with a concrete repro.
2. **C1 (gba-tests in Flycast)** — cheap once A1 works; converts the
   `parity` rows in Track B into pass/fail facts.
3. **C3/C4 (executable emit tests, host-compiled helpers)** — the highest
   -leverage automated coverage without hardware.
4. **B8 (literal pool)** — first perf item, only after a baseline FPS
   measurement on hardware/Flycast exists.
5. **E1 (release)** — when the smoke-test table is green.
