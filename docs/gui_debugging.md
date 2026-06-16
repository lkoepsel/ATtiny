# GUI debugging the ATtiny13A (gdbgui and Sublime/DAP)

Two optional graphical frontends for the existing Bloom + avr-gdb debug flow in
[`bloomandgdb.md`](bloomandgdb.md). Both are *frontends over the same stack* —
neither replaces Bloom, which remains the Linux-only debugWire ↔ GDB server.

```
  frontend  ──►  avr-gdb  ──►  Bloom  ──►  ATMEL-ICE / SNAP  ──►  ATtiny13A
 (gdbgui or                (debugWire ↔            (debugWire over RESET)
  Sublime+DAP)              GDB server,
                            Linux only)
```

Bloom manages the debugWire DWEN fuse automatically (`manage_dwen_fuse_bit:
true`) — there's no avrdude `PROGRAMMER_TYPE` to set for debugging; the debug
tool (SNAP) is configured in `bloom.yaml` (see `bloomandgdb.md`). Resource
ceiling is unchanged by any GUI: 64 bytes RAM,
debugWire offers 0 hardware breakpoints, software breakpoints rewrite flash.

## Which one?

| | **gdbgui** (Option A) | **Sublime + DAP** (Not an Option) |
|---|---|---|
| Frontend | Browser | Sublime Text 4 editor |
| Drives gdb via | GDB/MI | DAP (`avr-gdb --interpreter=dap`) |
| Min avr-gdb version | any modern (MI) | **≥ 14** |
| View on Mac, run on Pi | **Native** — web server on the LAN | Awkward (editor wants local access) |
| Setup friction | Low (`pip install gdbgui`) | Higher (custom adapter registration not found) |
| Registers / memory / disasm | Built-in panes | Depends on adapter UI |
| In-editor | No | Yes |

For the Mac-edits / Pi-debugs split here, **Option A (gdbgui) is the only one to try**: lower friction, no GDB-version gate, and the browser model is a natural fit for debugging on the Pi while sitting at the Mac. It also runs standalone on the Linux box.

---

## Option A — gdbgui (recommended)

A browser-based GDB frontend (`pip install gdbgui`) that launches a small web
server and drives gdb over GDB/MI. Shows source, registers, a memory viewer, and
disassembly — well suited to the `asm_*` assembly examples.

### Two deployment scenarios

- **Remote view (primary):** gdbgui + avr-gdb + Bloom all run on the **Pi**; you
  open the UI in a browser **on the Mac** at `http://<pi-ip>:5000`.
- **Standalone:** everything on the Linux box, browser local (`127.0.0.1:5000`).

### The shared `.gdbinit`

gdbgui auto-loads gdb's init files, so the project's existing `.gdbinit` does the
connect-and-flash work — no separate script is needed. The only requirement is
that its **TUI commands are guarded**, because the terminal TUI is incompatible
with the GDB/MI interpreter gdbgui uses (and with DAP). Guarding lets the *same*
file work in a plain terminal session *and* under gdbgui.

Keep this at `~/.gdbinit` — gdb (and the gdb that gdbgui launches) auto-loads the
home init unconditionally. Its relative paths (`file main.elf`, `load`, and the
`make`/`load` in `cll`) resolve against gdb's **working directory**, not the
init file's location, so the only requirement is to **start gdb/gdbgui from the
example directory** (`cd examples/asm_blink` first). A per-directory `./.gdbinit`
would also work but needs `set auto-load local-gdbinit on` plus a safe-path
entry — more friction, so the home init is preferred.

```gdb
set confirm off
set pagination off
set history save on
set history size 10000
set history filename ~/.gdb_history

file main.elf
target remote :1442
load
set listsize 0

# TUI is terminal-only; skip it silently under gdbgui/DAP (MI interpreter).
python
import gdb
try:
    gdb.execute("set tui compact-source on")
    gdb.execute("tui focus cmd")
except gdb.error:
    pass
end

define cll
make
load
l
end

define mrc
mon reset
continue
end

define td
tui disable
end

define te
tui enable
end
```

In a terminal, this enables the TUI as before; under gdbgui the `gdb.error` is
caught and the rest runs normally. Adjust `:1442` if `bloom.yaml` uses a
different GDB server port.

### First-time setup (on the Linux/Pi box, where Bloom lives)

1. **Install gdbgui** (isolated, so it doesn't disturb system Python):
   ```bash
   pipx install gdbgui        # or: python3 -m pip install --user gdbgui
   gdbgui --version           # confirm it runs
   ```
2. **Ensure the `.gdbinit` above is at `~/.gdbinit`** (the existing home init).
3. **Confirm the SNAP debug tool is set in `bloom.yaml`**; Bloom enables the
   DWEN (debugWire) fuse on connect — see `bloomandgdb.md`.

### Each debug session

Run from the example directory (so `.gdbinit`'s relative paths resolve):

1. **Build with debug symbols** — repo default is `-Og -ggdb3` (see CLAUDE.md):
   ```bash
   cd examples/asm_blink
   make complete            # produces main.elf with DWARF
   ```
2. **Start Bloom** in the project (debugWire). It opens the GDB server and halts
   the core on connect. Leave it running in its own terminal.
3. **Launch gdbgui** from the example directory. `.gdbinit` does `file` +
   `target remote` + `load`, so don't pass the ELF on the command line:
   - **Remote view** (browse from the Mac — expose on the LAN):
     ```bash
     gdbgui --host 0.0.0.0 --port 5000 --gdb-cmd avr-gdb
     ```
   - **Standalone** (browse locally on the Linux box):
     ```bash
     gdbgui --gdb-cmd avr-gdb
     ```
   If gdb's local-init auto-load is disabled in your setup, point at it
   explicitly instead: `--gdb-cmd "avr-gdb -x .gdbinit"`.
4. **Open the UI.**
   - Remote: on the Pi run `hostname -I` for its address; on the Mac browse to
     `http://<pi-ip>:5000`.
   - Standalone: browse to `http://127.0.0.1:5000` (gdbgui usually opens it for
     you).
5. **Debug.** gdbgui has already attached and flashed via `.gdbinit`, with the
   core halted. Set breakpoints in the gutter, then `continue`/step; inspect
   registers, memory, and disassembly in the panes. Your defines still work from
   gdbgui's console — `mrc` (reset + continue) and `cll` (rebuild + reload).

### Verify / confirm on first run

- `avr-gdb` MI works with gdbgui (any modern avr-gdb — no version gate).
- The port in `.gdbinit` (`:1442`) matches `bloom.yaml`.
- The core halts on attach (expected with debugWire) so you attach already
  stopped.

### Security note

`--host 0.0.0.0` exposes gdbgui — which can execute gdb commands — on the LAN.
Use only on a trusted network. gdbgui supports basic auth (`--auth-file`, or
`--user`/`--password`) and HTTPS (`--key`/`--cert`) if you want to lock it down.

### Optional: a `make gdbgui` convenience target

A per-example target could wrap the session launch (start gdbgui from the example
directory with `avr-gdb`). Worth adding only once the manual flow is proven.

---

## Explored and rejected: Sublime Text 4 via DAP

An in-editor DAP frontend (daveleroy `sublime_debugger` driving `avr-gdb
--interpreter=dap`) was investigated but **not adopted**: it requires avr-gdb ≥ 14
*and* a custom stdio adapter for GDB that the Debugger package doesn't provide,
and the editor-on-Mac / Bloom-on-Pi split makes it less convenient than gdbgui's
web view.
