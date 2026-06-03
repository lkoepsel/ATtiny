# Plan: Merge C and Assembly into a Single Code Base

Status: **decided** — ready to execute. Decisions locked (2026-06-03):

- **Link model:** auto-detect from presence of `.c` sources (override via `FREESTANDING`).
- **Naming:** prefix assembly examples with `asm_`.
- **`Makefile.asm`:** delete outright once verified.

## 1. Goal

Collapse the two parallel trees into one code base:

- One examples directory holding both C/mixed and pure-assembly examples.
- One root `Makefile` that builds either kind (retire `Makefile.asm`).
- One `Library/` holding C headers, C sources, asm headers, and asm sources
  (this is **already** the case — see §3).

End state: a developer copies any example folder, runs the same `make` targets,
and the build "does the right thing" whether the example is C, assembly, or a
mix of both.

## 2. Current State (as surveyed)

| Aspect | C examples | Assembly examples |
|---|---|---|
| Directory | `examples/` (24 dirs) | `asm_examples/` (8 dirs) |
| Root Makefile | `Makefile` | `Makefile.asm` |
| Entry source | `main.c` (+ optional `.S` via `ASM_LIBS`) | `main.S` |
| Link model | **with** avr-libc runtime, `--gc-sections`, `.map` | **freestanding**: `-nostartfiles -nostdlib` |
| Vector table | provided by avr-libc / C runtime | hand-written in `main.S` |
| Assemble flags | `-g` only | `-g -Wa,--gdwarf-2 -MMD -MP` (dep tracking) |
| `size` target | `objdump -Pmem-usage` | `avr-size -C` (hand-assembled ELF lacks deviceinfo) |
| `.hex` rule | `-j .text -j .data -O ihex` | `-O ihex -R .eeprom` |

**Already shared (no work needed):**
`Library/` contains both worlds today —
asm sources `registers.S`, `serial.S`, `sysclock.S`, `eeprom.S`;
C-facing headers for those asm routines `serial_asm.h`, `sysclock_asm.h`,
`eeprom_asm.h`; and the C header `ATtiny.h`. The C `Makefile` already links
shared `.S` files into C builds via `ASM_LIBS` (e.g. `examples/ticks`).

**The one real obstacle:** the two *link models*. A pure-asm example must link
freestanding (its `main.S` owns the vector table); a C/mixed example must link
with the runtime. The merged Makefile has to choose per example.

## 3. Key Design Decision — How the Makefile Picks the Link Model

**Recommended: auto-detect from the presence of `.c` sources, with an override.**

- If the example has **no `.c` files**, it is *freestanding* (pure asm) →
  link `-nostartfiles -nostdlib`, size via `avr-size -C`.
- If the example has **any `.c` file**, it is a *runtime* build (C or C+asm) →
  link with the runtime, `--gc-sections`, `.map`, size via `objdump -Pmem-usage`.

This matches the survey perfectly: every asm example is `main.S`-only; every C
example has `main.c`. No per-example annotation is required. An explicit escape
hatch is still provided for edge cases:

```make
# In a local Makefile, force the model regardless of sources:
#   FREESTANDING = 1   → no runtime (asm-style link)
#   FREESTANDING = 0   → link with runtime
# If unset, it is auto-detected from whether any .c sources exist.
```

**DECIDED: auto-detect.** Zero changes to most local Makefiles; the
`FREESTANDING` override remains available for edge cases.

## 4. Target Directory Layout & Naming

Merge `asm_examples/*` into `examples/`. Collisions and near-collisions must be
resolved:

| `asm_examples/` | Collides with | Note |
|---|---|---|
| `softserial` | `examples/softserial` | direct name clash |
| `blink_pwm` | `examples/blink_pwm` | direct name clash |
| `blink` | `examples/blink_asm` | **`blink_asm` is C-with-inline-asm**, not assembly — naming is already ambiguous |
| `blink_nodelay` | `examples/blink_wo_delay` | conceptual twin |
| `sysclock` | `examples/ticks` | conceptual twin |
| `blink_word`, `debug`, `skeleton` | — | no clash |

**DECIDED: prefix `asm_`.** No renames of existing C examples, sorts the
assembly set together, no clash with the existing `blink_asm` (C) example. The
`git mv` list for Phase 2:

| From | To |
|---|---|
| `asm_examples/blink` | `examples/asm_blink` |
| `asm_examples/blink_nodelay` | `examples/asm_blink_nodelay` |
| `asm_examples/blink_pwm` | `examples/asm_blink_pwm` |
| `asm_examples/blink_word` | `examples/asm_blink_word` |
| `asm_examples/debug` | `examples/asm_debug` |
| `asm_examples/softserial` | `examples/asm_softserial` |
| `asm_examples/sysclock` | `examples/asm_sysclock` |
| `asm_examples/skeleton` | `examples/asm_skeleton` |

## 5. Unified Makefile Design (sketch)

Extend the existing root `Makefile` (do **not** start from `Makefile.asm`) so
the richer C pipeline stays intact and gains a freestanding branch:

```make
SOURCES     = $(wildcard *.c)
ASM_SOURCES = $(wildcard *.S) $(ASM_LIBS)

# Auto-detect link model unless the local Makefile forces it.
ifeq ($(strip $(SOURCES)),)
  FREESTANDING ?= 1
else
  FREESTANDING ?= 0
endif

ifeq ($(FREESTANDING),1)
  LDFLAGS  = -nostartfiles -nostdlib
  SIZECMD  = $(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf
else
  LDFLAGS  = -Wl,-Map,$(TARGET).map -Wl,--gc-sections
  SIZECMD  = $(OBJDUMP) -Pmem-usage $(TARGET).elf
endif
```

Other merges:

- **Assembly rule**: adopt `Makefile.asm`'s dependency-tracking assemble rule
  (`-Wa,--gdwarf-2 -MMD -MP`) for *both* models so `.S` header edits trigger
  rebuilds. Add `-include $(DEPS)`.
- **`size` target**: run `$(SIZECMD)`.
- **`.hex` rule**: keep the C form (`-j .text -j .data`); confirm it produces a
  byte-identical hex for the asm examples (it should — they have no `.eeprom`).
  Verify in Phase 4 before deleting `Makefile.asm`.
- Keep all existing C-only targets (`static`, `stack`, `flash_eeprom`, fuse
  targets) — they're inert for asm examples.

## 6. Phased Plan with Division of Labor

Legend: **[Claude]** = I can do it · **[You]** = needs you (hardware,
decisions, judgment).

### Phase 0 — Decisions & branch
- **[You]** Resolve the two **[DECISION]** points (§3 link model, §4 naming).
- **[You]** Confirm I may create a working branch (e.g. `merge-c-asm`).
- **[Claude]** Once approved, create the branch and commit the current
  in-progress `Library/registers.S` change (or stash it) so the merge starts
  from a clean tree.

### Phase 1 — Unified Makefile (no moves yet)
- **[Claude]** Implement the §5 design in root `Makefile`.
- **[Claude]** Add a temporary smoke test: build one representative of each kind
  *in place* — a pure-asm example via a one-line local Makefile pointing at the
  new root `Makefile`, and confirm existing C examples still build unchanged.
- **[You]** Spot-check that a freshly built asm `.hex` matches the
  `Makefile.asm`-built `.hex` for the same example (diff the hex). This is the
  go/no-go gate for retiring `Makefile.asm`.

### Phase 2 — Move the examples
- **[Claude]** `git mv` each `asm_examples/<name>` into `examples/` under the
  chosen naming convention (preserves history).
- **[Claude]** Rewrite each moved local `Makefile` to `include $(DEPTH)Makefile`
  (and fix `DEPTH` if Option B subfolders are chosen).
- **[Claude]** Move `asm_examples/README.md` and `asm_examples/skeleton/` into
  their new homes; keep skeleton as the canonical asm template.

### Phase 3 — Build-everything verification
- **[Claude]** Add/centralize a `clean_all` + "build all" sweep that runs
  `make complete` in every `examples/` folder and reports failures. Run it.
- **[Claude]** Fix any include-path or `ASM_LIBS` breakage exposed by the sweep.
- **[You]** Review the build report; flag any example whose size/behavior
  changed unexpectedly.

### Phase 4 — Hardware verification (your bench)
- **[You]** Flash a representative set on real silicon and confirm behavior:
  - a pure-asm example (e.g. asm `blink`),
  - a C+asm example (`ticks` / `softserial`),
  - a plain-C example (`blink_avr`).
  Hardware, the programmer, and fuse state are only on your bench, so this gate
  is yours.
- **[Claude]** Address anything that builds but misbehaves, from your report.

### Phase 5 — Retire the old pipeline & update docs
- **[Claude]** Delete `Makefile.asm` outright (decided).
- **[Claude]** Update `CLAUDE.md`: collapse the two-Makefile table, rewrite the
  "Assembly Examples" and "Build System" sections, fix the example-structure
  description.
- **[Claude]** Update `docs/assembly.md`, `docs/README.md`, and the root
  `README.md` references to `asm_examples/` / `Makefile.asm`.
- **[Claude]** Merge `asm_examples/README.md`'s per-example index into the main
  example index.
- **[You]** Final review of prose for tone/accuracy, then approve the merge to
  `main`.

## 7. Verification Checklist (definition of done)

- [ ] `make complete` succeeds in **every** `examples/` folder.
- [ ] Asm-example `.hex` output is byte-identical to the pre-merge
      `Makefile.asm` output (Phase 1 gate).
- [ ] One example of each kind flashes and runs correctly on hardware.
- [ ] `make size`, `make disasm`, `make clean` work for both kinds.
- [ ] No remaining references to `asm_examples/` or `Makefile.asm` in docs,
      `CLAUDE.md`, `README.md`, or `.vscode/`.
- [ ] `git log --follow` shows preserved history on moved examples.

## 8. Risks & Rollback

- **Wrong link model selected** → asm example links in the C runtime, bloats
  past 1 KB or double-defines the vector table. *Mitigation*: the Phase 1 hex
  diff gate; the `FREESTANDING` override.
- **`r8`/`r9` reservation**: the C `Makefile` forces `-ffixed-r8 -ffixed-r9`
  for the asm sysclock tick counter (see `docs/sysclock_regpair.md`). This now
  applies to *all* C/mixed builds in the merged tree — confirm no plain-C
  example relies on those registers. *Likely fine* (already the default today),
  but call it out in review.
- **Lost git history** on moves → always `git mv`, never delete+add.
- **Rollback**: all work is on the `merge-c-asm` branch; `main` is untouched
  until Phase 5 approval.

## 9. Resolved Decisions

1. Link model: **auto-detect** from presence of `.c` (override via `FREESTANDING`). ✓
2. Naming: **prefix `asm_`** for moved assembly examples. ✓
3. `Makefile.asm`: **delete outright** after the Phase 1 hex-diff gate passes. ✓
4. `examples/blink_asm` (C-with-inline-asm): **left as-is** — not renamed.
