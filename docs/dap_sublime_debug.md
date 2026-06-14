# Debugging the ATtiny13A from Sublime Text 4 via DAP (experimental)

This is an **optional GUI frontend** for the existing Bloom + avr-gdb debug flow
documented in [`bloomandgdb.md`](bloomandgdb.md). It uses the Debug Adapter
Protocol (DAP) so breakpoints, stepping, variable/watch panes, and the call
stack appear inside Sublime Text 4 instead of the avr-gdb command line.

> **Status:** experimental / not yet wired up. The architecture and config shape
> below are settled; a few machine-specific items still need confirming on the
> Linux (Raspberry Pi) box — see the checklist at the end.

## It does not replace Bloom

DAP is a *frontend over the same stack*, not a second debug path:

```
Sublime Text 4 + Debugger pkg   (DAP client)
        │  Debug Adapter Protocol
avr-gdb --interpreter=dap        (GDB's native DAP, requires GDB >= 14)
        │  GDB remote serial protocol  (target remote localhost:1442)
Bloom                            (debugWire <-> GDB server, Linux only)
        │  debugWire over RESET pin
ATMEL-ICE / SNAP -> ATtiny13A
```

Bloom is still required and still Linux-only — it is the thing that drives the
ATMEL-ICE/SNAP over debugWire and exposes the GDB server that avr-gdb connects
to. Fuses must already be set for debugWire (`PROGRAMMER_TYPE = atmelice_dw`,
see `bloomandgdb.md`).

## Is it worth it?

Modest, real **ergonomic** value (stay in the editor, visual breakpoints) but
**not a capability gain**. The ATtiny13A's resources cap the payoff: 64 bytes
RAM, debugWire offers roughly one hardware breakpoint, and software breakpoints
rewrite flash (wear + slow). Worth setting up only if driving avr-gdb from the
terminal is your actual pain point.

## Config sketch

Sublime's Debugger package (daveleroy `sublime_debugger`) reads VS Code-style
configs from a `.sublime-project` file:

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

- `attach` + `target` is the correct pattern for a remote GDB server. GDB's DAP
  passes the `target` string straight to `target remote`. On a debugWire
  connect, Bloom halts the core, so avr-gdb attaches already stopped — then set
  breakpoints and continue.
- The adapter behind `"type": "gdb"` must launch `avr-gdb --interpreter=dap`.
- Set `program` to the `main.elf` of the example being debugged.

GDB DAP request parameters (for reference):
- **attach:** `program` (= `file` command), and one of `target` / `pid` / `coreFile`.
- **launch:** `program`, `args`, `cwd`, `env`, `stopOnEntry`,
  `stopAtBeginningOfMainSubprogram`.

## Workflow

1. Start Bloom in the project on the Linux box (debugWire); it opens the GDB
   server on its configured port.
2. In Sublime, run the debug configuration. The Debugger package spawns
   `avr-gdb --interpreter=dap`, which connects to Bloom via `target remote`.
3. The MCU is halted on connect — set breakpoints, then continue/step.

## To confirm on the Linux (Raspberry Pi) box

1. **avr-gdb >= 14** — native DAP requires it. Check `avr-gdb --version`.
2. **Custom adapter registration** — the one genuinely uncertain piece. The
   Debugger package documents only its pre-packaged adapters (LLDB, Node,
   Python, …), not GDB. Confirm how it registers a custom stdio adapter that
   runs `avr-gdb --interpreter=dap`, and match its name to `"type"` above.
3. **`bloom.yaml`** — confirm the GDB server host/port (default `1442`) and that
   debugWire fuses are set.
4. **`program` path** — point at the real `main.elf` for the target example.
