# Easy, high-impact fixes — audit 2026-09-10

Audited checkout: `2f25bfc`. This is a source audit and implementation plan, not a claim of runtime validation. No emulator code was changed. Scope: save/config paths, savestate loading, build entry points, CI, and existing validation evidence; not a full CPU correctness audit.

## Recommended first batch

Effort estimates include focused verification and assume a working build environment.

| Order | Fix | Evidence and impact | Effort | Acceptance check |
|---|---|---|---|---|
| 1 | Close config files on every path and reject failed reads | `gui.c:720` and `gui.c:775`: both config loaders close files only inside the valid-size branch. Wrong-size files leak handles; the global loader also returns success for them. Reads are unchecked before applying stack-buffer contents. | 1–2 hours | Repeated wrong-size loads do not grow open handles; failed/short reads retain defaults and return failure; valid configs still load. |
| 2 | Make direct Dreamcast builds select the ELF target | `dc/Makefile` includes KOS rules before declaring `all`; `scripts/dc-build.sh` explicitly documents that plain make selects KOS `subdirs` and builds nothing. README still recommends plain `make`. Set an explicit default goal before the include. | Under 1 hour | In a clean checkout, `make -C dc` produces `gdC.elf`; the Docker helper still works. |
| 3 | Report save failures and retain pending backup data | `memory.c:1934`, `1984`, `3341`; `gui.c:837`, `865`, `1114`: backup/config writes ignore write/close results; savestate save returns no status; automatic backup clears its pending countdown even when saving fails. Users can lose progress without feedback. | 0.5–1 day | Unwritable destination, failed write, and failed close report failure; no false success or preview refresh; backup remains pending with bounded retries and no per-frame warning spam. Valid writes succeed. |
| 4 | Correct storage promises and validation baseline | README describes runtime writes under `/cd/gbaDC/`; startup selects that directory and the inspected save paths use ordinary files. Disc storage is read-only. `scripts/flycast-test.md:58` prescribes 32 MB RAM, while README targets stock 16 MB hardware. | 1–2 hours for docs; runtime check separate | Clearly document current persistence limitations; distinguish 32 MB observations from a pending 16 MB run. Record exact commit, image hash, RAM setting, and results for the stock-memory smoke test. |
| 5 | Make CI cover development branches and retain its build | Both workflows only run pushes for `dreamcast`, `cursor/**`, and `claude/**`; a `codex/**` push gets no run until a PR targets `dreamcast`. The Dreamcast workflow checks the ELF exists but does not upload it. | 1–2 hours | Add `codex/**` or adopt all-branch push coverage; add manual dispatch and upload the built ELF. Verify one run exposes a downloadable binary with commit identity. |

For item 3, normalize I/O success carefully: stdio macros return an item count, while PSP macros return byte counts. Do not globally reinterpret either convention. Distinguish “no backup needed” from a failed save. A nonfatal menu message is appropriate; the existing fatal-error exit flow is not.

Item 3 makes failures visible; it does not implement persistent Dreamcast storage. VMU support requires separate work on file packaging, capacity, save sizes, and recovery. Treat it as a larger feature, not a save-indicator tweak.

## Next focused reliability fix

**Reject incomplete savestates before changing emulator state (roughly 1–2 days).**

`memory.c:3259` seeks past the thumbnail/time header without checking the result, then deserializes directly into live emulator state through unchecked reads. A truncated file can leave a mixture of old and restored state. The menu exits after calling the loader regardless of success (`gui.c:1125`).

First derive the serialized size from the actual serializer and the target ABI; do not assume the fixed `savestate_write_buffer[506947]` defines a portable format. Validate the complete payload before applying it, propagate read errors, keep the menu open on failure, and restore audio state on every exit. Test empty/truncated files, missing files, valid round trips, and failure after the initial read. Confirm rejected input leaves the running game unchanged. Full versioning, pointer-free serialization, and atomic replacement of existing saves are follow-on work.

## Reconcile the existing roadmap

- `ROADMAP.md` says nothing has been confirmed in Flycast, but the June 12 debug/performance reports record Tekken Advance gameplay. Preserve that observation while noting it used 32 MB RAM, an older commit, and local changes. It does not establish current-checkout or stock-hardware correctness.
- `scripts/smoke-test-results.md` still has an empty results table. Do not fill unperformed tests with passes or treat its “known-good” environment as a completed checklist.
- Promote persistence reliability above the existing D4 “VMU save indicator” item: storage support and error handling must precede success indicators.
- README's full-speed dynarec claim exceeds the supplied performance report, which explicitly says guest FPS was not measured. Qualify that claim until measured on the intended hardware.

## Defer from the easy-fix batch

Literal pools, register allocation, BIOS HLE, broad ARM semantic changes, and replacing zlib need targeted evidence and larger validation efforts. Host x86 build repair is useful but involves 32-bit assembly/linking and platform-specific flags (`x86/Makefile` includes `-mconsole`); it is not a one-line portability fix.

After the first batch, add one focused behavioral test around actual production save/config code, rather than another source-string contract. Existing emitter encoding tests exercise emission, but many other tests only check source text and cannot prove failure handling works.

## Verification limitations

The worktree was clean at audit start. No tests or Dreamcast builds were executed: `make` and `gcc` were unavailable on PATH, common local compiler paths were absent, and WSL distribution enumeration returned access denied. Historical reports and statements of CI success are prior evidence only. Run the host suite and KOS cross-build when implementing changes, then perform the documented Flycast checks with the RAM setting recorded.

Suggested delivery: first PR for config loading, default build target, and CI; second PR for save-error propagation with behavioral tests; documentation corrections alongside the corresponding fixes; then savestate validation. Rough total for the first five items: 1–2 working days plus emulator validation.

## Implementation follow-up

The first five items are implemented locally: checked config reads and unconditional closure; an explicit Dreamcast default goal; checked save writes/close results with nonfatal feedback, exit acknowledgement, and bounded backup retries; corrected storage and stock-memory documentation; and development-branch/manual CI triggers with an ELF artifact. Failed savestate saves now return failure and skip the menu preview refresh.

Validation: production-function fault-injection tests pass with MSVC, covering invalid/missing/short-read configs, open/write/close failures, handle closure, audio unlock, successful saves, retry recovery, and retry throttling despite new backup writes. Eleven existing host test programs also pass with MSVC (the build-contract test uses Windows popen/pclose aliases). The emitter encoding test requires GCC extensions and could not compile with MSVC. The KOS cross-build, live CI run, and 16 MB Flycast/UI checks remain unverified because the Docker engine is unavailable and no target run was performed. The fault-injection harness uses simulated I/O and serialization; it does not validate the complete savestate format or rendering.

Savestate input validation and VMU persistence remain follow-on work. Existing saves are still written directly, so a short write may damage the destination; atomic replacement is not part of this batch.
