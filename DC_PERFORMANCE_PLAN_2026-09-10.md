# Dreamcast full-speed plan — 2026-09-10

Performance audit of the SH-4 dynarec and the Dreamcast runtime, with a phased
plan to reach full speed. Every cost in this document is measured, not
estimated.

**Baseline, measured 10 September 2026 in Flycast 2.7 at stock 16 MB and
200 MHz.** These are the first frame rates this port has produced.

| Workload | Frames per second | Share of the 59.73 target |
|---|---|---|
| Homebrew title screen, fully resident | 53.9 | 90% |
| Super Puzzle Fighter II, mean over 65 s of attract-mode play | 23.2 | 39% |
| Super Puzzle Fighter II, worst sample | 8.5 | 14% |

The homebrew control is the important half: the disc, video and flip path
sustain about 54 fps, so the deficit on commercial code is recompiler
throughput rather than presentation. Full detail and the sample set are in
[scripts/smoke-test-results.md](scripts/smoke-test-results.md).

Flycast's SH-4 timing is approximate rather than cycle-exact, so treat these as
first-order estimates of Dreamcast speed, not hardware measurements.

Companion documents: [ROADMAP.md](ROADMAP.md) (correctness and release tracks),
[DYNAREC_AUDIT_2026-09-10.md](DYNAREC_AUDIT_2026-09-10.md) (correctness audit 4).
This document supersedes roadmap rows **B8** and **B9** and adds findings that
were not previously tracked.

---

## 1. How the numbers were obtained

**Build.** The current worktree cross-compiles cleanly with the CI toolchain
(`einsteinx2/dcdev-kos-toolchain:gcc-9__v2.0.0`, KOS default `-O2
-fomit-frame-pointer`). Section sizes are from `sh-elf-size`.

**Emission costs.** `dc/sh4_emit.h` compiles on the host against the shim
already used by `tests/sh4_emit_encoding_test.c`. Each emitter macro was run
against a counting buffer and the 16-bit instructions it wrote were counted
directly. Constant-materialisation averages sample 16,777,216 distinct values.

### Memory image

| Section | Bytes |
|---|---|
| `.text` | 1,430,396 |
| `.rodata` | 68,332 |
| `.data` | 14,242 |
| `.bss` | 3,943,032 |
| **Loaded image** | **about 5.26 MB** |

Add the 4 MB resident gamepak buffer and the port occupies roughly 9.3 MB of
the Dreamcast's 16 MB before KOS, SDL, stack and heap.

### Emitted-code costs, in 16-bit SH-4 instructions

| Emitter | Cost |
|---|---|
| 32-bit constant, mean over sampled values | 9.0 |
| 32-bit constant, worst case | 14 |
| Helper call: address plus `jsr` plus delay `nop` | 12 |
| Constant shift, mean over amounts 0 to 31 | 15.5 |
| Constant shift, worst case (31 bits) | 31 |
| Load or store of ARM `r0` through `r15` | 1 |
| Load or store of N, Z, C, V or CPSR | 3 |

### Whole translated instructions

| ARM / Thumb instruction | SH-4 instructions emitted |
|---|---|
| Thumb `ADDS Rd,Rs,Rn` | 15 |
| Thumb `LSL Rd,Rs,#5` | 14 |
| Thumb `LDR Rd,[Rb,#0x20]` | 15 |
| Thumb `STR Rd,[Rb,#0x20]` | 22 |
| ARM `ADD r0,r1,r2` | 15 |
| ARM `ADD r0,r1,r2,LSL #7` | 22 |
| Direct branch block exit | 30 |
| Indirect branch exit | 13 |

A hand-written SH-4 sequence for `ADD r0,r1,r2` in gpSP's memory form is four
instructions: two loads off the register base, one `add`, one store. The
backend currently emits fifteen and takes a function call through a
pipeline-flushing `jsr` and `rts` pair.

The Dreamcast SH-4 has an **8 KB instruction cache**. At roughly 300 bytes per
translated basic block the whole instruction cache holds about 27 blocks.
Shrinking emitted code is a speed lever in its own right, independent of
instruction count.

---

## 2. Findings

Ranked by expected impact. Each names the code that produces the cost.

### F1 — The entire translation cache is flushed on every newly translated block

`cpu_threaded.c:2824` and `cpu_threaded.c:2911` call
`translate_invalidate_dcache_region(<cache base>, <current write pointer>)`
after each top-level translation. That reaches `dc/sh4_stub.c:365`, which runs
`dcache_flush_range` followed by `icache_flush_range` over the whole used
region, plus a further 256 bytes.

KOS walks these ranges one 32-byte cache line at a time. A ROM cache that has
grown to 512 KB means about 16,384 line operations per cache, twice, for every
new block. The cost grows as the cache fills, so it is worst exactly when a
game is streaming new code: level loads, cut-scene entry, and every recovery
after a self-modifying-code flush of the RAM cache.

Only the newly emitted range needs flushing. Capture the write pointer before
translation and flush from there. Because the current broad flush also covers
branch backpatch sites, narrowing it requires an explicit single-line flush at
each patch site in the `generate_branch_patch_*` family.

This finding is not in the existing roadmap and is the cheapest large win
available.

### F2 — Every data-processing instruction is an out-of-line C call

`arm_data_proc`, `arm_data_proc_test`, `arm_data_proc_unary`,
`thumb_data_proc` and their variants in `dc/sh4_instr.inc` all end in
`generate_function_call(execute_<name>)`. The callees are one-line functions:

```c
u32 function_cc execute_and(u32 rm, u32 rn) { return rn & rm; }
u32 function_cc execute_add(u32 rm, u32 rn) { return rn + rm; }
```

Every `AND`, `ORR`, `EOR`, `BIC`, `ADD`, `SUB`, `RSB`, `MOV` and `MVN` in every
translated block pays twelve instructions of address materialisation, a `jsr`,
a wasted delay slot, and an `rts` return. Thumb is worse, because nearly every
Thumb ALU instruction sets flags and so always lands in a helper.

The non-flag operations map one-to-one onto SH-4 instructions and should be
emitted inline. The logical flag-setting operations need only N and Z, which
cost two instructions each on SH-4 (`tst` then `movt`, and `shll` then `movt`).
Only the arithmetic flag-setting forms genuinely need `addc` and `addv`
sequences, and those are still cheaper inline than a call.

### F3 — 32-bit constants are built a byte at a time

`SH4_EMIT_LOAD_IMM` at `dc/sh4_emit.h:337` materialises a constant with a
`mov`, then repeated `shll8` plus `add` per byte. Measured mean is 9.0
instructions and the worst case is 14.

SH-4 has `mov.l @(disp,PC),Rn`, which loads a 32-bit literal in one instruction
from a pool within 1020 bytes. The codebase already uses this form once, in the
long-branch veneer at `dc/sh4_emit.h:306`, so the mechanism is proven. A
general literal pool turns a helper call from twelve instructions into three
and shrinks every immediate, every PC load, and every cycle-count update.

Constraints worth designing for up front: the 8-bit displacement gives roughly
1 KB of reach, so pools must be emitted and re-anchored periodically inside
long blocks, and a PC-relative load must never be placed in a branch delay
slot.

### F4 — Constant shifts emit one instruction per bit shifted

`generate_shift_left`, `generate_shift_right`,
`generate_shift_right_arithmetic` and `generate_rotate_right` at
`dc/sh4_emit.h:453` are literal loops:

```c
do { u32 _sh = (imm_val); while(_sh--) SH4_EMIT_SHLL1(SH4_IREG(ireg)); } while(0)
```

A shift by 31 emits 31 instructions. The mean over shift amounts 0 to 31 is
15.5. SH-4 provides `shll2`, `shll8`, `shll16` and their logical-right
counterparts, and `shad` and `shld` for arbitrary amounts from a register. Any
constant shift should cost at most three instructions. Thumb's `LSL`, `LSR` and
`ASR` immediate forms are among the most common opcodes in compiled GBA code,
and ARM shifted operands appear throughout data processing.

### F5 — The flag slots sit outside the load/store displacement window

`SH4_EMIT_LOAD_REG` uses `mov.l @(disp,Rn)`, whose 4-bit displacement reaches
60 bytes. ARM `r0` through `r15` land inside that window and cost one
instruction. `REG_N_FLAG` through `REG_CPSR` are `reg[16]` through `reg[20]`,
at byte offsets 64 to 80, so each access falls back to building the offset in a
scratch register: three instructions instead of one.

Every conditional instruction evaluates one or two flags, and every
flag-setting helper stores four. Pinning a second base register at `&reg[16]`
brings `reg[16]` through `reg[31]` into the window at displacement 0 to 60, an
exact fit. The registers `r3` and `r8` through `r11` are unused by the emitter,
and `r8` through `r11` are callee-saved under the SH-4 ABI, so they survive
helper calls the same way `r12` and `r13` already do. The dispatch trampoline
at `dc/sh4_stub.c:100` sets the new base alongside the existing ones.

### F6 — ARM `LDM` and `STM` are interpreted at run time

`arm_block_memory` at `dc/sh4_instr.inc:745` emits nothing but a call to
`execute_arm_block_memory`, which re-decodes the opcode and walks the register
list on every execution. The Thumb equivalent, `thumb_block_memory`, already
unrolls the list at translate time. ARM block transfers carry function
prologues and epilogues in ARM-mode code, so the gap matters for ARM-heavy
titles.

### F7 — Every branch wastes its delay slot

`SH4_EMIT_JSR`, `SH4_EMIT_JMP` and the branch fillers all emit `nop` into the
delay slot. At call sites the instruction immediately before the call is
usually an argument load that could move into the slot instead.

### F8 — Loads and stores have no inline fast path

`arm_access_memory_load` and the Thumb equivalent always call
`execute_load_*`. The helper body is a short `memory_map_read` probe:

```c
if(((address & 0xF0000003) == 0) && (map = memory_map_read[address >> 15]))
  dest = *(u32 *)((u8 *)map + (address & 0x7FFF));
```

Inlining that probe and calling out only on the slow path removes the `jsr` and
`rts` pipeline cost from the most frequent instruction class in GBA code. This
is worth doing after F3, because a literal pool already cuts the call itself to
three instructions.

### F9 — The resident ROM buffer is 4 MB against a 16 MB game

`init_gamepak_buffer` at `memory.c:3111` allocates a fixed 4 MB on Dreamcast.
Grand Theft Auto Advance is 16 MB, so three quarters of it is demand-paged in
32 KB pages from the open disc file during play. Measured static usage is 5.26
MB, so raising the buffer toward 8 MB looks feasible, though it must be
verified against a real 16 MB configuration rather than assumed.

Two caveats. Real GD-ROM seek latency is tens of milliseconds, and neither
redream nor Flycast reproduces it, so paging cost is invisible under emulation
and only appears on hardware. And `savestate_write_buffer` at `memory.c:3376`
is a 495 KB static array that is live only while saving; making it dynamic
returns half a megabyte.

### F10 — There is no frame-rate instrumentation on Dreamcast

`synchronize()` at `main.c:873` reports timing through `SDL_WM_SetCaption`,
which does nothing on Dreamcast. The same function also runs `sprintf` with a
float conversion every frame, which is not free on SH-4.

The baseline above was obtained externally, by reading Flycast's own frame
counter, which works only because the Dreamcast build presents exactly one flip
per emulated GBA frame. That is enough to establish a starting number, but it
cannot separate emulation time from render time, cannot run on hardware, and
cannot report translation counters. An on-target overlay is still needed.
`print_string` already draws into the GBA framebuffer for the pause menu, so it
is straightforward.

### F11 — The build uses stock KOS optimisation flags

KOS supplies `-O2 -fomit-frame-pointer` and nothing else. `dc/Makefile` already
exposes a `GPSP_EXTRA_CFLAGS` hook. The `-O3` setting and link-time
optimisation affect the C-side scanline renderer, the sound mixer and the
helper functions, not dynarec output, so this is a secondary lever to measure
rather than assume.

### F12 — Automatic frameskip is the default but does nothing on Dreamcast

`current_frameskip_type` defaults to `auto_frameskip` with a value of 4, but
the non-PSP `synchronize()` clears `skip_next_frame` and then only sets it
again inside an `if(current_frameskip_type == manual_frameskip)` branch. The
automatic path exists only in the PSP build.

So the Dreamcast never drops a frame to catch up. It renders and presents every
frame however long that takes, which is why the measured rate falls to 8.5 fps
rather than holding a higher rate with skipped frames. Wiring up the automatic
path, or defaulting to manual frameskip, would improve perceived smoothness
immediately and independently of everything else here.

This is also what makes the baseline readable: presented frames equal emulated
frames, so Flycast's counter is the GBA frame rate. Any frameskip work should
keep a way to read both numbers.

### F13 — Grand Theft Auto Advance stops with a recompiler fatal error

The 16 MB title boots and reaches its title sequence, then halts:

```
Dynarec translation failed:
bad jump 1a3019f8 (3000154) (0)
```

The requested block address is not a valid GBA region, so the region switch in
`block_lookup_address_*` falls through to its `default:` case. The CPU was
executing in IWRAM at `0x03000154`, meaning an indirect branch produced a
garbage target.

Two smaller titles do not reproduce it, and one of those is also demand-paged,
so ROM paging alone does not explain it. This is a correctness defect, not a
performance one, but it blocks using the largest and most demanding available
title as a benchmark.

---

## 3. Plan

### Phase 0 — Make speed observable (partly done)

A reproducible build-and-launch recipe now exists (section 4), and a baseline
has been recorded for two titles. What remains is on-target instrumentation, so
that later phases can be judged without an external emulator.

1. On-screen frame time and frame rate overlay on Dreamcast, drawn with
   `print_string`, toggled from the pause menu. Replace the per-frame
   `sprintf` float conversion with integer formatting.
2. An unthrottled benchmark mode: disable the `synchronize_flag` pacing, run a
   fixed number of frames from a fixed save state, and report mean and worst
   frame time.
3. Counters for translated blocks, cache flushes and bytes emitted, shown on
   the same overlay. F1 and F3 are both judged by these.
4. Separate emulation time from render time, so the unmeasured video and audio
   cost noted at the end of this section can be quantified rather than guessed.

**Exit gate:** the Super Puzzle Fighter II baseline of 23.2 fps mean is
reproducible from the on-target overlay, without reading Flycast's counter.

### Phase 1 — Cheap, high-confidence wins

Independent of each other; all are local changes with existing host-test
coverage to extend.

| Item | Finding | Change |
|---|---|---|
| 1.1 | F1 | Flush only the newly emitted range; add per-site flushes at branch patch points |
| 1.2 | F4 | Emit constant shifts with `shll2`, `shll8`, `shll16`, `shad` and `shld` |
| 1.3 | F5 | Pin a second register base at `&reg[16]`; set it in the dispatch trampoline |
| 1.4 | F11 | Measure `-O3` and link-time optimisation through `GPSP_EXTRA_CFLAGS` |
| 1.5 | F12 | Make automatic frameskip work on the Dreamcast path, or default to manual |

Item 1.5 changes perceived smoothness rather than throughput, and it changes
how the baseline reads, so land it with the overlay from Phase 0 reporting
emulated and presented frames separately.

**Exit gate:** Phase 0 numbers improve, and `make -C tests test` plus the
existing emitter and helper suites still pass.

### Alongside — F13

The Grand Theft Auto Advance fatal error is a correctness defect and does not
belong to any performance phase, but it blocks benchmarking on the largest
available title. Isolating it needs a register dump at the fatal-error site and
a trace of the last translated block, which is Phase 0 instrumentation work.

### Phase 2 — The literal pool

F3, on its own, because it touches every emitter and needs its own test pass.

1. Add a per-block literal pool with deduplication, anchored so every
   `mov.l @(disp,PC)` stays within reach.
2. Re-anchor when a block outgrows the displacement range, and forbid pool
   loads in delay slots.
3. Route `SH4_EMIT_FUNCTION_CALL` and `SH4_EMIT_LOAD_IMM` through it, keeping
   the existing one-instruction small-constant path.
4. Revisit `SH4_ARM_MAX_EMIT_BYTES_PER_INSN` (roadmap B7); pooling changes the
   worst-case emission bound in both directions.
5. Extend `tests/sh4_emit_simulator.h` to execute pool loads so the encoding
   suite covers them.

**Exit gate:** helper calls measure three instructions; emitted bytes per block
drop measurably on the Phase 0 counters.

### Phase 3 — Inline the hot instruction classes

| Item | Finding | Change |
|---|---|---|
| 3.1 | F2 | Inline non-flag ALU operations |
| 3.2 | F2 | Inline logical flag-setting forms, N and Z only |
| 3.3 | F2 | Inline arithmetic flag-setting forms with `addc`, `addv`, `subc` and `subv` |
| 3.4 | F8 | Inline the memory-map probe for loads and stores, calling out on miss |
| 3.5 | F7 | Fill branch delay slots with the preceding argument load |

Item 3.3 is the one here with real correctness risk. The September 10 audit
found four flag bugs in these exact semantics, so each inlined form needs a case
in `tests/sh4_helpers_behavior_test.c` executed against the same reference
calculations already used there.

**Exit gate:** `ADD r0,r1,r2` emits four instructions; the behaviour suite
passes with the inlined forms cross-checked against the helper implementations.

### Phase 4 — Structural work, only if Phase 0 numbers still demand it

| Item | Finding | Notes |
|---|---|---|
| 4.1 | F6 | Unroll ARM `LDM` and `STM` at translate time, as Thumb already does |
| 4.2 | F9 | Raise the resident ROM buffer; free `savestate_write_buffer` |
| 4.3 | — | Block-level register allocation (roadmap B9) |
| 4.4 | — | Return-address cache for indirect branches |

Item 4.3 is the largest remaining lever and the largest project. Do not start it
before Phases 1 to 3 are measured, because the call-heavy code they remove is
what makes register allocation look necessary in the first place.

### What this does not address

Video and audio cost is unmeasured. The GBA scanline renderer in `video.c` and
the mixer in `sound.c` run as ordinary C and may hold a significant share of
the frame once dynarec cost falls. Phase 0's frame-time counters should
separate emulation time from render time so that this can be answered rather
than guessed.

---

## 4. Running it

The port boots and plays commercial GBA games under Flycast. Recipe below.

### Build

The Docker cross-build produces `dc/gdC.elf`. On Windows, Git Bash rewrites the
container's working directory, so `scripts/dc-build.sh` needs
`MSYS_NO_PATHCONV=1`:

```bash
MSYS_NO_PATHCONV=1 docker run --rm -v "F:/GitHub/gPSPDC:/src" -w /src/dc   einsteinx2/dcdev-kos-toolchain:gcc-9__v2.0.0 make all
```

### Disc image

`mkdcdisc`, which `dc/dc.sh` requires, is **not** in the CI container.
`scramble`, `mkisofs` and `cdi4dc` are, and together they build a CDI that both
Flycast and redream parse. Three details matter, each of which cost a failed
boot to find:

- **The binary must be scrambled.** An unscrambled `1ST_READ.BIN` boots as far
  as the Sega licence screen and then spins forever.
- **The session offset must be 11702**, matching where `cdi4dc` places the data
  track. Get it wrong and the bootstrap reports a sector read miss and cannot
  find the boot file.
- **`gba_bios.bin` goes at the disc root**, not under `gbaDC/`; the emulator
  loads it from `/cd/gba_bios.bin`.

```bash
MSYS_NO_PATHCONV=1 docker run --rm -v "F:/GitHub/gPSPDC:/src" -w /src/dc   einsteinx2/dcdev-kos-toolchain:gcc-9__v2.0.0 sh -c '
    rm -rf /tmp/d && mkdir -p /tmp/d
    cp cd/gba_bios.bin /tmp/d/
    cp -r cd/gbaDC /tmp/d/
    sh-elf-objcopy -R .stack -O binary gdC.elf /tmp/g.bin
    scramble /tmp/g.bin /tmp/d/1ST_READ.BIN
    mkisofs -C 0,11702 -V GPSPDC -G cd/IP.BIN -joliet -rock -l -o /tmp/d.iso /tmp/d
    /opt/toolchains/dc/kos/utils/img4dc/cdi4dc/cdi4dc /tmp/d.iso /src/dc/gpspdc.cdi'
```

The ROM to load on boot is named in `dc/cd/gbaDC/autoload.txt`. ROMs go in
`dc/cd/gbaDC/`, and both the BIOS and any `.gba` files are already gitignored.

### Run

```bash
flycast.exe F:/GitHub/gPSPDC/dc/gpspdc.cdi
```

Flycast is the emulator to use. redream v1.5.0 has no Dreamcast boot ROM here,
falls back to its own bootstrap, and fails immediately on a homebrew disc;
Flycast's REIOS handles it. To read the frame rate, add to Flycast's `emu.cfg`:

```
[config]
rend.ShowFPS = yes
```

Because the Dreamcast build presents one flip per emulated GBA frame, Flycast's
`F:` counter is the GBA frame rate. That stops being true if F12 is fixed and
frameskip starts dropping frames.

One note on the launch command: redream and Flycast are Dreamcast emulators and
take a disc image, so neither can open a `.gba` file. The GBA ROM goes onto the
disc.

### Known rough edge

The emulator draws its 240x160 output into the top-left corner of the
Dreamcast's framebuffer rather than scaling it. The scaling code in
`flip_screen` is commented out. Cosmetic, but it makes the window look broken
at first glance.
