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

Fuses must already be set for debugWire (`PROGRAMMER_TYPE = atmelice_dw`, see
`bloomandgdb.md`). Resource ceiling is unchanged by any GUI: 64 bytes RAM,
debugWire offers ≈1 hardware breakpoint, software breakpoints rewrite flash.

## Which one?

| | **gdbgui** (Option A) | **Sublime + DAP** (Option B) |
|---|---|---|
| Frontend | Browser | Sublime Text 4 editor |
| Drives gdb via | GDB/MI | DAP (`avr-gdb --interpreter=dap`) |
| Min avr-gdb version | any modern (MI) | **≥ 14** |
| View on Mac, run on Pi | **Native** — web server on the LAN | Awkward (editor wants local access) |
| Setup friction | Low (`pip install gdbgui`) | Higher (custom adapter registration unconfirmed) |
| Registers / memory / disasm | Built-in panes | Depends on adapter UI |
| In-editor | No | Yes |

For the Mac-edits / Pi-debugs split here, **Option A (gdbgui) is the one to try
first**: lower friction, no GDB-version gate, and the browser model is a natural
fit for debugging on the Pi while sitting at the Mac. It also runs standalone on
the Linux box.

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

Place this `.gdbinit` in the example directory (paths are relative, so gdb must
be started from that directory):

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
2. **Add the `.gdbinit` above** to the example directory you'll debug
   (e.g. `examples/asm_blink/.gdbinit`).
3. **Confirm fuses are set for debugWire** (`atmelice_dw`) per `bloomandgdb.md`.

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

## Option B — Sublime Text 4 via DAP (experimental)

An in-editor frontend using the Debug Adapter Protocol, so breakpoints, stepping,
and variable/watch panes appear inside Sublime. Uses GDB's **native DAP**
(requires avr-gdb ≥ 14).

```
Sublime Text 4 + Debugger pkg → avr-gdb --interpreter=dap → target remote → Bloom → ATtiny13A
```

### Config sketch

Sublime's Debugger package (daveleroy `sublime_debugger`) reads VS Code-style
configs from a `.sublime-project`:

```json
{
  "folders": [{ "path": "." }],
  "debugger_configurations": [
    {
      "name": "ATtiny13A — Bloom + avr-gdb (DAP)",
      "type": "gdb",
      "request": "attach",
      "program": "${project_path}/examples/asm_blink/main.elf",
      "target": "localhost:1442"
    }
  ]
}
```

- `attach` + `target` is correct for a remote GDB server; GDB's DAP passes
  `target` straight to `target remote`. The MCU halts on connect, so you attach
  already stopped.
- The adapter behind `"type": "gdb"` must launch `avr-gdb --interpreter=dap`.
- GDB DAP params — **attach:** `program` plus one of `target`/`pid`/`coreFile`;
  **launch:** `program`, `args`, `cwd`, `env`, `stopOnEntry`,
  `stopAtBeginningOfMainSubprogram`.

### Open items to confirm on the Linux box

1. **avr-gdb ≥ 14** (native DAP). Check `avr-gdb --version`.
2. **Custom adapter registration** — the one genuinely uncertain piece. The
   Debugger package documents only its pre-packaged adapters, not GDB; confirm
   how it registers a custom stdio adapter running `avr-gdb --interpreter=dap`
   and match its name to `"type"` above.
3. **`bloom.yaml`** GDB host/port; **`program`** path to the real `main.elf`.

Note the cross-machine friction: with the editor on the Mac and Bloom on the Pi,
DAP is less convenient than gdbgui's web view — a reason Option A is preferred
here.
