# zed-avr-asm

A minimal [Zed](https://zed.dev) extension that adds syntax highlighting for
GNU-assembler (`avr-as`) AVR assembly — the dialect used by the `asm_examples/`
in this repo (`.section`, `.include`, `@progbits`, `lo8()`, `;` comments).

## What it provides

- Syntax highlighting for `.asm` and `.inc` files
- Jump-to-label / symbol search (`Ctrl-Shift-O`) via the outline panel

It is **Tier 1** only: highlighting is *structural*, not semantic — a mistyped
mnemonic still highlights as an instruction. No hover docs, completion, or
diagnostics (those would need a real AVR language server).

## Install (development mode)

1. Open Zed.
2. Command palette → **`zed: install dev extension`**.
3. Select this `zed-avr-asm/` directory.

Zed fetches and compiles the tree-sitter grammar on first install (needs `git`).
Open any `.asm` file in `asm_examples/` to confirm — the status bar should read
**AVR Assembly**.

## Editing the highlight rules

`languages/avr-asm/highlights.scm` and `outline.scm` reload when you save them.
Changes to `extension.toml` or `config.toml` require re-installing the dev
extension.

## Notes

- The grammar is the generic `RubixDev/tree-sitter-asm`, pinned by commit. There
  is no AVR-specific tree-sitter grammar; the generic one parses GAS structure
  (labels, mnemonics, directives, comments) well enough for highlighting.
- `.inc` files such as `tn13Adef.inc` contain C-preprocessor guard lines
  (`#ifndef`, `#define`, `#endif`). The generic grammar parses these as line
  comments; `highlights.scm` recolors any `#`-prefixed line as a keyword so it
  reads as a directive. The `.equ` register definitions parse correctly.
