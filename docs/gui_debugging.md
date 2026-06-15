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

### Implementation plan

Run these **on the Linux/Pi box** (where Bloom lives) unless noted.

1. **Install gdbgui** (isolated, so it doesn't disturb system Python):
   ```bash
   pipx install gdbgui        # or: python3 -m pip install --user gdbgui
   gdbgui --version           # confirm it runs
   ```

2. **Build the target with debug symbols.** The repo default is already
   `-Og -ggdb3` (see CLAUDE.md). In the example directory:
   ```bash
   make complete              # produces main.elf with DWARF; or `make flash`
   ```
   gdbgui needs `main.elf` for source/symbol info; Bloom has already programmed
   flash, so no `load` is required at debug time.

3. **Create a one-line gdb connect script** so the session auto-attaches to
   Bloom. In the example directory, add `bloom.gdbinit`:
   ```
   target remote localhost:1442
   ```
   (Use the host/port from `bloom.yaml`; `1442` is Bloom's default.)

4. **Start Bloom** in the project (debugWire); it opens its GDB server and halts
   the core on connect. Leave it running.

5. **Launch gdbgui** pointing at avr-gdb, the connect script, and the ELF:
   - Remote view (expose on the LAN):
     ```bash
     gdbgui --host 0.0.0.0 --port 5000 \
            --gdb-cmd "avr-gdb -x bloom.gdbinit examples/asm_blink/main.elf"
     ```
   - Standalone (local only — omit `--host`):
     ```bash
     gdbgui --gdb-cmd "avr-gdb -x bloom.gdbinit examples/asm_blink/main.elf"
     ```

6. **Open the UI.** On the Pi: get its address with `hostname -I`. On the Mac:
   browse to `http://<pi-ip>:5000`. Set breakpoints in the gutter, then
   `continue`/step; inspect registers, memory, and disassembly in the panes.

### Verify / confirm on first run

- `avr-gdb` MI works with gdbgui (any modern avr-gdb — no version gate).
- Bloom's GDB port matches `bloom.gdbinit` (check `bloom.yaml`).
- The core halts on attach (expected with debugWire) so you attach already
  stopped.

### Security note

`--host 0.0.0.0` exposes gdbgui — which can execute gdb commands — on the LAN.
Use only on a trusted network. gdbgui supports basic auth (`--auth-file`, or
`--user`/`--password`) and HTTPS (`--key`/`--cert`) if you want to lock it down.

### Optional: a `make gdbgui` convenience target

A per-example target could wrap step 5 (start gdbgui with the right ELF and
`bloom.gdbinit`). Worth adding only once the manual flow is proven.

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
