; Labels — e.g. reset_handler:, main_loop:
(label
  [(ident) (word)] @label)

; CPU registers — r0..r31
(reg) @variable.builtin

; Assembler directives — .section, .include, .text, .equ, ...
(meta
  kind: (_) @keyword)

; Instruction mnemonics — ldi, rjmp, sbi, reti, ...
(instruction
  kind: (_) @function)

; Named constants
(const
  name: (word) @constant)

; Comments — ; line comments and /* block */
[
  (line_comment)
  (block_comment)
] @comment

; C-preprocessor guard lines (#ifndef/#define/#endif) in .inc files.
; The generic grammar parses these as line comments; recolor so they
; read as directives rather than comments. Last match wins.
((line_comment) @keyword
  (#match? @keyword "^#"))

; Literals
(int) @number
(float) @number
(string) @string

; Operand keywords
[
  "byte"
  "word"
  "dword"
  "qword"
  "ptr"
  "rel"
  "label"
  "const"
] @keyword

; Operators
[
  "+"
  "-"
  "*"
  "/"
  "%"
  "|"
  "^"
  "&"
] @operator

; Punctuation
[
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

[
  ","
  ":"
] @punctuation.delimiter
