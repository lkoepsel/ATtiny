# Debugging the ATtiny13A — terminal TUI + Bloom Insight

The debug stack is unchanged from [`bloomandgdb.md`](bloomandgdb.md): *Bloom* is the
Linux-only *debugWire* ↔ *GDB* server, driven by *avr-gdb*.

```
  avr-gdb  ──►  Bloom  ──►  ATMEL-ICE / SNAP  ──►  ATtiny13A
 (TUI in a   (debugWire ↔            (debugWire over RESET)
  terminal)   GDB server,
              Linux only)
```

The recommended front end is **avr-gdb's built-in terminal TUI**, optionally
alongside **Bloom's Insight** GUI for registers/peripherals/memory. Two heavier
frontends — gdbgui (browser) and Sublime + DAP — were explored and **rejected**;
see [Explored and rejected](#explored-and-rejected) for why, so the dead ends
aren't re-walked.

Resource ceiling is unchanged by any front end: 64 bytes RAM, debugWire offers 0
hardware breakpoints (software breakpoints rewrite flash). Bloom manages the
debugWire DWEN fuse automatically (`manage_dwen_fuse_bit: true`) — there's no
avrdude `PROGRAMMER_TYPE` to set for debugging; the debug tool is configured in
`bloom.yaml` (see `bloomandgdb.md`).

---

## Recommended: avr-gdb TUI

avr-gdb's TUI is a curses split-screen (source window over the command window)
built into gdb — no extra install, no web server, no version gate. It's a natural
fit for the small `asm_*` and C examples and works identically on a Linux box or
over SSH to a Pi.

### The shared `.gdbinit`

gdb auto-loads `~/.gdbinit`, so it does the connect-and-flash work and brings the
TUI up automatically. Its relative paths (`file main.elf`, `load`, and the
`make`/`load` in `cll`) resolve against gdb's **working directory**, so the only
requirement is to **start gdb from the example directory** (`cd
examples/asm_blink` first).

Keep this at `~/.gdbinit`:

```gdb
set confirm off
set pagination off
set history save on
set history size 10000
set history filename ~/.gdb_history

file main.elf
target remote :1442
load
tbreak main

# Terminal TUI: source-over-command split, brought up automatically.
set tui compact-source on
tui enable
tui focus cmd

# Rebuild + reload, then redraw the TUI source window.
define cll
make
load
refresh
end

# Reset to 00 and run.
define mrc
mon reset
continue
end

# Toggle the TUI off / on (e.g. to paste, or read raw multi-line output).
define td
tui disable
end

define te
tui enable
end
```

Adjust `:1442` if `bloom.yaml` uses a different GDB server port.

`tbreak main` gives a one-shot stop at the start of a C example so the TUI opens
halted at your code. Assembly (`asm_*`) examples have no `main` symbol — swap it
for a temporary breakpoint on the reset handler or a known label, e.g.
`tbreak *0` (the reset vector) or `tbreak reset`. Use `tbreak` (temporary), not a
plain `break`, so it doesn't immediately re-trip at the current PC after `load`.

### TUI quick reference

| key / command | action |
|---|---|
| `Ctrl-x a`        | toggle TUI on/off (or use the `td` / `te` defines) |
| `Ctrl-x 2`        | cycle through layouts (src, asm, regs, split) |
| `layout asm`      | disassembly window — useful for the `asm_*` examples |
| `layout regs`     | add the register window |
| `Ctrl-x o`        | change the focused window (so arrows scroll it) |
| `Ctrl-p` / `Ctrl-n` | command history (arrows are captured by the TUI) |
| `Ctrl-l`          | redraw if the screen gets corrupted (`refresh` also works) |
| `Ctrl-c`          | interrupt a running core (Bloom honors the RSP break) |

### Each debug session

Run from the example directory (so `.gdbinit`'s relative paths resolve):

1. **Build with debug symbols** — repo default is `-Og -ggdb3` (see CLAUDE.md):
   ```bash
   cd examples/asm_blink
   make complete            # produces main.elf with DWARF
   ```
2. **Start Bloom** in the project (debugWire). It opens the GDB server and halts
   the core on connect. Leave it running in its own terminal.
3. **Launch avr-gdb** from the example directory:
   ```bash
   avr-gdb
   ```
   `.gdbinit` runs `file` + `target remote` + `load` + `tbreak main`, then brings
   up the TUI with the core halted at your code.
4. **Debug.** Set breakpoints, `continue`/`step`/`next`, inspect with `info
   registers`, `x`, `print`. `Ctrl-c` halts a running core. `mrc` resets and runs;
   `cll` rebuilds and reloads.

---

## Optional: Bloom Insight

Bloom ships **Insight**, a Qt GUI that attaches to the same debug session and
shows target registers, peripherals, memory, and GPIO state — the register/memory
inspection that a terminal TUI doesn't surface graphically. It runs alongside the
gdb TUI (gdb still drives execution; Insight is a live view of the target).

Enable it per environment in `bloom.yaml`:

```yaml
environments:
  snap_13a:
    # ...tool / target / server as in bloomandgdb.md...
    insight:
      activate_on_startup: true   # was false
```

Then start Bloom as usual; the Insight window opens with the session. See
`bloomandgdb.md` for the full Bloom configuration and the Insight feature notes
in Bloom's own docs.

---

## Explored and rejected

Both of the following were investigated against the Mac-edits / Pi-debugs split
and dropped. Recorded here so they aren't re-attempted.

### gdbgui (browser frontend)

A browser GDB frontend (`pip install gdbgui`, driving gdb over GDB/MI). It worked,
but wasn't worth the friction for a 1 KB MCU over debugWire:

- **Pause button is a no-op on remote targets.** gdbgui halts by signalling a
  *local* inferior process; over debugWire there is none (gdb is just an RSP
  client to Bloom), so you have to type `interrupt` anyway — the TUI's `Ctrl-c`
  is no worse.
- **No layout configuration.** gdbgui 0.15 has no flag/config to remove or
  rearrange panes; trimming the always-empty "Program Output" terminal (the
  ATtiny has no stdout) required hand-patching CSS *inside the pip venv's
  template* — an edit wiped by every `pip install --upgrade gdbgui`.
- **Coupled source views.** Its source pane and the GDB console's source text are
  both driven by gdb's frame output, so the duplicate listing in the console
  couldn't be quieted (`set print frame-info location` blanks the good pane too)
  without, again, a CSS hack.
- **Requires `set mi-async on`** in `.gdbinit` to function, which is irrelevant
  to (and clutter for) a terminal/TUI workflow.

Net: a curses TUI plus Insight covers the same ground (source, stepping,
registers, memory, disassembly) with zero install, no web server on the LAN, and
no per-upgrade patching. The gdbgui venv was at `gdb/` and has been removed (see
[Cleanup](#cleanup-removing-gdbgui) below).

### Sublime Text 4 via DAP

An in-editor DAP frontend (daveleroy `sublime_debugger` driving `avr-gdb
--interpreter=dap`) was investigated but never adopted: it requires avr-gdb ≥ 14
*and* a custom stdio adapter for GDB that the Debugger package doesn't provide,
and the editor-on-Mac / Bloom-on-Pi split makes it less convenient than even the
terminal TUI over SSH.

---

## Cleanup: removing gdbgui

gdbgui was installed as a self-contained uv venv at `gdb/` (≈32 MB, untracked /
never committed, no system footprint — `avr-gdb` is the system `/usr/bin/avr-gdb`
and is untouched). To remove it:

```bash
rm -rf gdb/
```

That's the only artifact; the CSS layout patch lived inside the venv's
`templates/gdbgui.html`, so it goes with it. Nothing to undo in git (the directory
was never tracked), and there's no `pip`/`pipx` entry to uninstall — the venv was
the whole install.
