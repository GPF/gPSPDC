# SH-4 dynarec audit and polish — 2026-09-10

Four reproduced correctness issues were fixed in `dc/sh4_helpers.c`; immediate-load emission was also shortened in `dc/sh4_emit.h`. This pass reviewed arithmetic and shifter helpers, their emitter call sites, masked CPSR writes, branch/veneer emission, and store/dispatch exits. It does not establish complete ARM compatibility or Dreamcast performance.

## Fixed findings

| Finding | Reproduction | Change |
|---|---|---|
| ADC/SBC/RSC carry and overflow flags mishandle carry-in edge cases | `ADCS 0, 0xffffffff` with C=1 should produce zero with C=1. `SBCS 0, 0` with C=0 should produce `0xffffffff` with C=0 and V=0. Existing helpers compute incorrect flags. | Capture incoming carry before modifying flags; account for equality when calculating carry/borrow; use operand/result sign bits for overflow. RSC shares the corrected SBC helper with reversed operands. |
| Register-specified shifts use the full shift register; rotates can shift C values by 32 or more | A shift register of `0x100` specifies zero shift, preserving carry. Nonzero rotates by multiples of 32 preserve the value and set carry from bit 31. | Mask register shifts to the low byte; normalize rotate counts before shifting and handle zero separately. Thumb register shifts share the corrected result/carry helpers and then update N/Z. |
| Masked CPSR writes lose live flags | With stale CPSR flags and live N=1/C=1, writing only CPSR control bits clears those live flags. Unmasking an IRQ can also save the wrong flags in SPSR_irq. | Collapse the live flag slots into CPSR before applying the write mask. Explicit writes to the flag field still replace flags normally. |
| Exception-return addresses can override the restored instruction state | An ARM return target ending in bit 0 can reach the dual dispatcher as a Thumb target. A pending IRQ can also receive an unaligned LR_irq. | Align the return PC according to restored SPSR.T before IRQ handling, then encode the dispatcher state bit. Sixteen ARM/Thumb/alignment/IRQ cases reproduced eight failures before the fix. |

Arithmetic expectations use independent signed/unsigned 64-bit reference calculations. Carry-in and no-borrow semantics were also cross-checked against the [GNU CGEN ARM7TDMI instruction descriptions](https://sourceware.org/cgen/gen-doc/arm-thumb-insn.html). No external implementation was copied.

## Emission polish

Immediate materialization now skips leading zero bytes during translation. Loading `0x80` takes two instructions instead of six; `0x100` takes two and `0x10000` takes three. The existing signed one-instruction path remains, and full-width constants retain their previous behavior. No literal pools or additional runtime scratch registers are introduced. This reduces generated code for small constants; it is not a measured FPS result.

## Regression coverage

`tests/sh4_helpers_behavior_test.c` includes the actual production helper implementation. A minimal KOS header shim and host memory/IRQ stubs allow compiling it without SDL or a Dreamcast runtime. The test exercises:

- Six arithmetic operations, both incoming carry values at boundary operands, and 10,000 deterministic operand pairs; checks result and all NZCV flags.
- All four register shift/rotate operations in ARM flag-preserving, ARM carry-updating, and Thumb NZC-updating variants; counts 0–511 and `0xffffffff`, with both carry values.
- CPSR control-only writes, flag-only writes, and flag preservation in the IRQ snapshot after an unmasking write.

The initial arithmetic/shift suite reproduced **3,218 failing checks**. The separate CPSR cases reproduced **two further failures**. After fixes, **159,283 checks pass** with the production helpers compiled using MSVC `/O2`.

The expanded helper suite and all **12 pre-existing host C test programs** pass (13 programs total). The emitter test gained a test-only MSVC compatibility macro for GCC-style declaration attributes; emitted code is unchanged. The build-contract test was compiled with Windows `popen`/`pclose` aliases. The new helper test is included in `make -C tests test` and its clean target.

The emitter suite now includes a small test interpreter for immediate loads and conditional branches. It executes **32,784 cases**: 2,048 constants across all 16 registers, plus both branch polarities and slot alignments for near and literal-veneer skips. It checks result values, preservation of other registers, and compact sequences for selected constants. Unsupported instructions, invalid control flow, and non-NOP branch delay slots fail the test; this is not a general SH-4 emulator.

## Remaining validation and scope

These tests execute C helpers on the host, inspect instruction encodings, and simulate selected emitted sequences. They do not execute translated blocks on an SH-4 or test real memory/IRQ side effects, bank switching, cache coherency, the dispatch ABI, or frame timing. The KOS cross-build and 16 MB Flycast/hardware smoke test remain required; the local Docker engine is unavailable.

No literal-pool, register-allocation, or cycle-accounting changes were made, and no hardware speedup is claimed. Block-memory alert/SMC handling, LDM exception-return semantics, and indirect-branch timing remain follow-up audit targets alongside the existing roadmap. Earlier save/build fixes remain in the worktree; this pass does not commit or publish them.
