# Debugging the ATtiny13A from Sublime Text 4 via DAP (experimental)

This is an **optional GUI frontend** for the existing Bloom + avr-gdb debug flow
documented in [`bloomandgdb.md`](bloomandgdb.md). It uses the Debug Adapter
Protocol (DAP) so breakpoints, stepping, variable/watch panes, and the call
stack appear inside Sublime Text 4 instead of the avr-gdb command line.

> **Status:** experimental. The architecture and config are settled and the
> toolchain prerequisites are confirmed on the Linux box (see the checklist at
> the end). The GDB adapter question is resolved: Sublime's Debugger package has
> **no built-in GDB adapter**, so you install the **Native Debug** adapter
> (`code-debug`), which provides `"type": "gdb"`. The remaining step is an
> end-to-end test against live hardware.

## It does not replace Bloom

DAP is a *frontend over the same stack*, not a second debug path:

```
Sublime Text 4 + Debugger pkg       (DAP client)
        │  Debug Adapter Protocol
Native Debug / code-debug           (DAP adapter; provides type: "gdb")
        │  GDB Machine Interface (MI), driving the gdb set by "gdbpath"
avr-gdb                             (gdbpath: avr-gdb)
        │  GDB remote serial protocol  (target remote :1442)
Bloom                              (debugWire <-> GDB server, Linux only)
        │  debugWire over RESET pin
ATMEL-ICE / SNAP -> ATtiny13A
```

Note the middle layer: the Debugger package ships no GDB adapter of its own.
Native Debug (`code-debug`) is the DAP adapter that exposes `"type": "gdb"`, and
it drives a real `gdb` over the **MI** interface — *not* GDB's own
`--interpreter=dap`. You point it at `avr-gdb` via the `"gdbpath"` field.

Bloom is still required and still Linux-only — it is the thing that drives the
ATMEL-ICE/SNAP over debugWire and exposes the GDB server that avr-gdb connects
to. With `manage_dwen_fuse_bit: true` in `bloom.yaml` (set for all environments
here), Bloom programs the **DWEN** fuse via ISP at session start and clears it on
a clean exit — you do **not** set debugWire fuses manually. See `bloomandgdb.md`
and the **debugWire gotchas** section below.

## Is it worth it?

Modest, real **ergonomic** value (stay in the editor, visual breakpoints) but
**not a capability gain**. The ATtiny13A's resources cap the payoff: 64 bytes
RAM, debugWire offers roughly one hardware breakpoint, and software breakpoints
rewrite flash (wear + slow). Worth setting up only if driving avr-gdb from the
terminal is your actual pain point.

## Config sketch

Sublime's Debugger package (daveleroy `SublimeDebugger`) reads VS Code-style
configs from the `debugger_configurations` array of a `.sublime-project` file.
This repo ships one at `ATtiny.sublime-project`, using **Native Debug**'s remote
GDB attach format:

```json
{
  "folders": [{ "path": "." }],
  "debugger_configurations": [
    {
      "name": "ATtiny13A — Bloom + avr-gdb (Native Debug)",
      "type": "gdb",
      "request": "attach",
      "executable": "${project_path}/examples/asm_blink/main.elf",
      "target": ":1442",
      "remote": true,
      "cwd": "${project_path}",
      "gdbpath": "avr-gdb",
      "valuesFormatting": "prettyPrinters"
    }
  ]
}
```

Native Debug GDB field reference (these names are *its* schema, not VS Code's
generic gdb / not GDB's native DAP):
- **`type: "gdb"`** — selects the Native Debug GDB adapter.
- **`request: "attach"` + `remote: true`** — connect to a running gdbserver
  rather than launching a local process. This is the Bloom case.
- **`target: ":1442"`** — the remote endpoint, passed to `target remote`. A bare
  `:1442` means localhost:1442 (Bloom's `avr_gdb_rsp` port in `bloom.yaml`).
- **`executable`** — the `.elf` with debug symbols (Native Debug uses
  `executable`, *not* `program`). Set per example being debugged.
- **`gdbpath: "avr-gdb"`** — use the AVR gdb instead of the system `gdb`.
- **`cwd`** — working directory for gdb.
- Optional: **`autorun`** (array of gdb commands run after connect, e.g.
  `["monitor reset"]`), **`stopAtConnect`**, **`valuesFormatting`**.

On a debugWire connect Bloom halts the core, so gdb attaches already stopped —
set breakpoints, then continue/step.

## One-time setup

1. **Install the Debugger package** (daveleroy `Debugger`) via Package Control.
2. **Install the GDB adapter**: command palette → `Debugger: Install Adapters`
   → choose **Native Debug** (`code-debug`, WebFreak001). This is what provides
   `"type": "gdb"`; the Debugger package has no GDB adapter of its own.
   - Native Debug needs **Node.js** on `PATH`; if Sublime can't find it, set the
     node path in the Debugger package settings. Install it minimally and
     optionally sandbox it — see *Node.js: install minimally & sandbox* below.
3. **Open the project**: Project → Open Project → `ATtiny.sublime-project` (open
   the project, not just the folder, so `debugger_configurations` is read).

## Node.js: install minimally & sandbox

Native Debug is a Node program, so this route needs the Node **runtime**. The
real "Node malware" threat is the **npm supply chain** (`npm install` resolving
untrusted dependency trees and running their `postinstall` scripts), *not* the
runtime itself. This setup avoids that: `Debugger: Install Adapters` downloads
Native Debug as a prebuilt, pinned artifact — there is no `npm install` step in
the workflow. So the highest-value precaution is simply: **don't install npm.**

### Minimal install (do this)

On CachyOS/Arch, `nodejs` and `npm` are *separate* packages and `nodejs` does
not depend on `npm`:

```bash
sudo pacman -S nodejs        # runtime only, signed distro package
# deliberately NOT: pacman -S npm
```

Use the distro package — never `nvm`, `curl | bash`, or unofficial binaries,
which bypass package signing. With no `npm`/`node-gyp`/global modules present,
there is no dependency-resolution step anywhere to exploit.

### Sandbox the adapter (optional, defense-in-depth)

Point the Debugger package's Node-path setting at `tools/node-confined` (shipped
in this repo) instead of `node`. It launches Node — and the `avr-gdb` it spawns —
inside **bubblewrap** (`bwrap`, already installed) so the adapter:

- cannot read `$HOME` secrets (`~/.ssh`, dotfiles, browser data) — `$HOME` is not
  bound into the sandbox;
- can only write inside the project directory; the rest of the filesystem is
  read-only;
- keeps loopback networking (so `avr-gdb` still reaches Bloom on
  `127.0.0.1:1442`) while PID/IPC/etc. are isolated.

Caveats: because `$HOME` is hidden, `~/.gdbinit` will not load inside the sandbox
(fine — under Native Debug the `.sublime-project` config drives gdb), and you
must confirm the Debugger Node-path override actually routes through the wrapper.
Prefer `bwrap` over `firejail` (firejail is setuid-root with a history of
privilege-escalation CVEs).

## Workflow

1. Build the example so the ELF matches what's flashed:
   `cd examples/asm_blink && make`.
2. Start Bloom with the SNAP environment on the Linux box:
   `bloom snap_13a`. It enters debugWire and opens the GDB server on `:1442`.
3. In Sublime, select the `ATtiny13A — Bloom + avr-gdb (Native Debug)`
   configuration and start debugging. Native Debug launches `avr-gdb`, which
   connects to Bloom via `target remote :1442`.
4. The MCU is halted on connect — set breakpoints, then continue/step.

## debugWire gotchas

The practical debugWire pitfalls (chip stuck in debugWire after an unclean exit,
SNAP not powering the target, RESET-pin hardware, full ISP wiring, breakpoint
budget) apply to this DAP flow exactly as they do to the terminal flow. They are
documented once in [`bloomandgdb.md`](bloomandgdb.md#debugwire-gotchas).

## Verified on the Linux box (2026-06-14)

1. ✅ **avr-gdb** — `avr-gdb` is **17.1** at `/usr/bin/avr-gdb`. (Native Debug
   drives it over MI, so GDB's native `--interpreter=dap` isn't required; any
   modern avr-gdb works.)
2. ✅ **`bloom`** — Bloom **v2.0.0** at `/usr/bin/bloom`; `bloom.yaml` sets the GDB
   server to `port: 1442` (matches `target: ":1442"` above). All environments set
   `manage_dwen_fuse_bit: true`, so Bloom handles the DWEN fuse automatically —
   no manual `atmelice_dw` step.
3. ✅ **`executable` path** — `examples/asm_blink/main.elf` exists; rebuild the
   example before debugging so the ELF matches the flashed image.
4. ✅ **GDB adapter route** — resolved: install **Native Debug** (`code-debug`),
   which supplies `"type": "gdb"` (no custom stdio-adapter registration needed).

## Still to confirm

- **Native Debug not yet installed** — Package Control currently lists only
  `AVR, Git, LSP, LSP-clangd, Package Control`. Install the `Debugger` package
  and then the Native Debug adapter (see *One-time setup*).
- **End-to-end test** — with hardware connected and `bloom snap_13a` running,
  confirm Native Debug connects via `target remote :1442`, halts, and that
  breakpoints/stepping work. The `target`/`remote`/`gdbpath` fields are from
  Native Debug's documented schema but haven't been exercised here yet.
- **Node.js for Native Debug** — the adapter requires Node on `PATH`; verify or
  set its path in the Debugger settings.
