# gPSPDC SH-4 Dynarec Performance Report - 2026-06-12

## Executive Summary

- Current runtime path is the SH-4 dynarec, not the interpreter.
- Fresh CDI `dc/gbapspDC-fresh.cdi` boots in Flycast and runs `Tekken Advance (USA).gba` into gameplay.
- Flycast stayed responsive during the run.
- No release-build in-emulator FPS counter or internal cycle telemetry is currently enabled, so this report separates confirmed runtime behavior from unmeasured performance.
- The biggest remaining dynarec performance lever in the roadmap is a literal-pool strategy for `SH4_EMIT_LOAD_IMM`; the next biggest is block-level register allocation.

## Test Context

- Date/time: 2026-06-12 22:38 America/New_York
- Branch: `dreamcast`
- HEAD: `6a589af38906518adc920cf689fe90676478cf92`
- Runtime CDI: `dc/gbapspDC-fresh.cdi`
- Runtime ROM: `dc/cd/gbaDC/Tekken Advance (USA).gba`
- BIOS: `dc/cd/gba_bios.bin`
- Live screenshot: `C:\Users\allen\AppData\Local\Temp\codex-shot-2026-06-12_22-35-49.png`

Artifact hashes:

```text
SHA256 dc/gbapspDC-fresh.cdi
6A8BDDC69920A59C8DC4487DB39AF4AAD583FF966CA59889AD0F043B00ACC04F

SHA256 dc/gdC.elf
448743074D1F873C52AADEF04869591C3208DBA37DCC802276A5AF14120C3F63
```

Flycast settings relevant to performance:

```text
Dynarec.Enabled = yes
Dreamcast.RamMod32MB = yes
FastGDRomLoad = no
Sh4Clock = 200
rend.Resolution = 480
rend.EmulateFramebuffer = no
rend.ThreadedRendering = yes
aica.BufferSize = 2822
```

## Validation Evidence

Host contract suite:

```text
make -C tests test
Result: pass
```

Dreamcast build check:

```text
./scripts/dc-build.sh all
Result: pass/current
```

Runtime observation:

- Fresh CDI boots.
- BIOS and ROM load through `/cd`.
- Tekken Advance reaches title screen.
- Input advances to an in-game match.
- Video renders correctly at the documented native GBA framebuffer size in the upper-left of the larger Dreamcast/Flycast surface.

## Host-Side Runtime Sample

This is a Flycast host-process sample, not a direct GBA FPS measurement.

```text
Sample duration: 25.145 seconds
Flycast CPU time delta: 9.000 seconds
Approx one-core equivalent: 35.8%
Working set range: about 254 MB to 280 MB
Responding: true for all samples
Window: Flycast - gbapspDC-fresh
```

Interpretation:

- The emulator process remained healthy during gameplay.
- Host CPU use was moderate on this PC.
- This does not prove Dreamcast hardware full speed. Flycast host CPU use includes Flycast's SH-4/PVR/AICA emulation overhead and does not expose the guest GBA framerate.

## Dynarec Architecture

Entry path:

- Dreamcast builds use `execute_arm_translate(execute_cycles)` unless `GPSP_DC_INTERPRETER` is defined.
- `execute_arm_translate()` extracts flags, looks up or translates the ARM block at `reg[REG_PC]`, captures the dispatch stack base, then jumps into emitted SH-4 code through `sh4_dispatch_block()`.

Dispatch model:

- Emitted code treats:
  - `r12` as the `reg` array base.
  - `r13` as the live cycle counter.
  - `r15` as the SH-4 stack pointer.
- `sh4_dispatch_block()` pins inputs to low caller-saved registers before overwriting `r12`, `r13`, and `r15`, avoiding allocator hazards.
- Blocks do not return normally; async exits and PC changes re-enter through `sh4_lookup_pc(cycles)`.

Translation caches on Dreamcast:

```text
ROM translation cache:  1,048,576 bytes
RAM translation cache:    262,144 bytes
BIOS translation cache:    65,536 bytes
Flush threshold:            1,024 bytes
```

Lookup and translation:

- BIOS/RAM use block tags embedded next to emulated memory.
- ROM uses `rom_branch_hash`, avoiding writes into ROM.
- Cache overflow triggers a cache flush and translation retry.
- Recursive translation retries are capped; Dreamcast fatal-errors instead of spinning forever.

Cycle accounting:

- `cycle_count` is accumulated while translating a block.
- `generate_cycle_update()` emits nothing when `cycle_count == 0`.
- Non-zero cycle updates subtract from the live SH-4 `r13` cycle budget.
- Calls to `sh4_update_gba()` update timers/video/audio/input and return a refreshed cycle budget.
- Idle-loop targets force hardware updates at configured branch PCs.

## Current Performance Strengths

- SH-4 dynarec is active and booting commercial gameplay.
- Direct mapped memory stores avoid helper calls when possible.
- Slow-path stores collapse flags before IO writes, preserving IRQ flag correctness without forcing broad interpreter fallback.
- `CPU_ALERT_IRQ` dispatches directly with the live cycle counter instead of billing a full update interval.
- Conditional far-skip support avoids corrupt short branches in large conditional runs.
- Zero-cycle update guard avoids needless emitted load/sub sequences.
- Dreamcast build uses native 240x160 GBA framebuffer, minimizing video scaling cost.
- Menu/video mode changes were previously reduced: Dreamcast stays at the larger SDL surface instead of switching modes during gameplay/menu transitions.

## Current Performance Costs

- Every ARM register access still goes through the `reg` array via `r12`; there is no block-level ARM-register allocation into SH-4 registers.
- `SH4_EMIT_LOAD_IMM` can materialize a 32-bit constant in up to 14 SH-4 instructions. Helper calls embed function addresses frequently, so this bloats hot translated blocks.
- Helper-heavy instructions, IO writes, SMC checks, CPSR/SPSR paths, cheats, and block memory operations still cross back into C.
- ROM paging from disc can affect large titles; current test ROM was 8 MB and not a large-ROM paging stress test.
- Idle-loop elimination is per-game and requires a matching `game_config.txt` entry. The config currently has 96 active `idle_loop_eliminate_target` entries, but not every tested ROM is covered.
- Indirect branches intentionally retain upstream-parity timing behavior where pending translate-time cycle count is not flushed in all cases.

## Known Dynarec Performance Risks

From `ROADMAP.md`:

- `B8`: PC-relative literal pool for constants is the largest remaining dynarec speed lever.
- `B9`: Block-level register allocation would reduce repeated `reg[]` memory traffic, but is a larger project.
- `A6`: Idle-loop profile still needs validation under the SH-4 dynarec.
- `A3`: 30+ minute soak tests are still needed on ARM-heavy and Thumb-heavy games.
- Large ROM paging needs separate testing with a 16 MB or 32 MB title.

Correctness risks that can affect perceived performance:

- Any SMC-heavy title can flush RAM translation cache often.
- Missing idle-loop targets can make games run miserably slowly despite correct execution.
- Large helper-call density can make some games CPU-bound before video/audio become the bottleneck.

## Measurement Gaps

Not yet measured:

- True guest FPS.
- Frame pacing jitter.
- Audio underrun count.
- Translation-cache occupancy over time.
- Number of translated blocks per second.
- Cache flush count during gameplay.
- Idle-loop hit count.
- Dynarec vs interpreter side-by-side performance on the same current build.
- Real Dreamcast hardware speed.

Why: the current release build does not expose this telemetry in a clean, persistent log, and Flycast serial logging is disabled in the active config.

## Recommended Next Instrumentation

1. Add a `GPSP_DC_PERF_TRACE` build mode that logs once per second:
   - `frame_ticks`
   - skipped/rendered frame counts
   - `rom_translation_ptr - rom_translation_cache`
   - `ram_translation_ptr - ram_translation_cache`
   - translation flush counts
   - idle-loop hits
   - SMC flushes

2. Add a lightweight on-screen debug overlay behind `GPSP_DEBUG`:
   - FPS or frames per second window
   - frameskip state
   - ROM/RAM translation cache use

3. Run a three-ROM baseline:
   - Small direct GBA ROM: `Tekken Advance (USA).gba`
   - Large 16 MB ROM: `F-Zero - GP Legend (USA).gba`
   - Another 16 MB ROM: `Mario Tennis - Power Tour (USA, Australia) (En,Fr,De,Es,It).gba`

4. Compare with an interpreter CDI only after telemetry exists, otherwise the comparison is mostly visual and subjective.

## Bottom Line

The SH-4 dynarec is functional enough to boot a fresh CDI and run Tekken Advance into gameplay in Flycast. The current performance evidence is positive but qualitative: responsive Flycast, successful gameplay, moderate host CPU use, and green contract/build checks.

The report cannot honestly claim full-speed Dreamcast performance yet. To make that claim, the next step is explicit guest FPS/cache/idle-loop telemetry plus a small benchmark matrix across one small ROM and at least two large ROMs.
