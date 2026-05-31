Good direction — the higher-level helpers (soft_string_write, soft_int16_write, soft_readLine, soft_pgmtext_write) are just composition over
  char_write / char_read, so swapping the two primitives gets you all the timing benefits with minimal churn.

  The work breaks into four pieces: making the asm conform to the avr-gcc ABI, exposing the symbols to C, teaching the C Makefile about .S files,
  and updating the one existing asm caller. Here's what each entails.

  1. The AVR-GCC ABI contract

  This is the rule set the asm has to obey to be C-callable:

  - Arguments: first byte arg in r24 (not r17). Second in r22, third in r20, fourth in r18. 16-bit values go in pairs r25:r24, r23:r22, etc.
  - Return: byte return in r24. 16-bit in r25:r24.
  - Call-saved (must preserve if touched): r2–r17, r28, r29.
  - Call-clobbered (free for the callee): r18–r27, r30, r31, r0.
  - Zero register: r1 must equal 0 on entry and exit. Don't clobber it; if you must, restore with eor r1, r1 before returning.

  Audit Library/serial.S against that and you'll see the mismatches:
  - char_write reads the char from r17 → must read from r24 (or mov r24 into a scratch first thing).
  - char_read returns the char in r17 → must return in r24.
  - It uses r17, r18, r19, r20. r17 is call-saved under the ABI — either stop touching it (use r18 instead) or push/pop it. r18–r20 are
  call-clobbered, free.
  - The T-flag framing-error status from char_read has no C equivalent — pick one: return uint16_t with status in the high byte, or take a uint8_t
   *status arg, or just drop it (the existing C code ignored it anyway).

  2. Exposing the symbols

  Three lines at the top of the relevant routines:

  .global init_serial
  .global char_write
  .global char_read

  (Without .global, the linker can't see them — they'd be local to the object.) timer_delay stays private; no .global needed.

  Then a header for C to import from:

  // serial_asm.h
  #pragma once
  #include <stdint.h>

  void    init_serial(void);
  void    char_write(uint8_t c);
  uint8_t char_read(void);

  The name char_write is fine — AVR-GCC does no name mangling. (If you ever build the C code as C++, wrap the declarations in extern "C".)

  3. The C Makefile needs to assemble .S

  The root C Makefile currently globs only *.c. avr-gcc handles .S just fine — you only need to add the source pattern, a pattern rule, and the
  objects to the link line:

  SOURCES_C := $(wildcard *.c $(LIBDIR)/*.c)
  SOURCES_S := $(wildcard *.S $(LIBDIR)/*.S)
  OBJECTS   := $(SOURCES_C:.c=.o) $(SOURCES_S:.S=.o)

  %.o: %.S Makefile
        $(CC) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

  A few things to note:
  - The existing CPPFLAGS already adds -I$(LIBDIR), so #include "registers.S" resolves from inside serial.S without further work.
  - You don't want CFLAGS on the .S rule — -std=gnu99 -Wundef -Werror etc. don't apply to assembly. Use only CPPFLAGS + TARGET_ARCH.
  - The no_lib branch wildcard would skip $(LIBDIR)/*.S. Decide whether to make this example LIBRARY != no_lib (so the Library .S gets compiled)
  or copy serial.S into the example directory the same way the C examples copy soft_serial.c. The "copy into example" route fits the existing
  C-example convention.

  4. The existing asm caller has to switch to r24 too

  This is the one part that affects code you've already got working. asm_examples/serial/main.S calls char_write today with the char in r17
  (and serial.S is #included, not linked). Once serial.S becomes ABI-compliant:

  - Every rcall char_write in main.S needs the char in r24 beforehand.
  - Every rcall char_read reads its result from r24 instead of r17.

  Small, mechanical change — but you'll want to do it in the same commit as the asm rework so the asm example doesn't break.

  A bigger question lurking here: should asm_examples/serial/main.S stop #include-ing serial.S and instead let it be linked (the same way
  the C build will)? That would make Library/serial.S a true shared library object — one file, one build artifact, two callers. It needs
  Makefile.asm to also pick up $(LIBDIR)/*.S and link the resulting .o. Worth it long-term; not required to start.

  A suggested order of attack

  1. Branch first — this touches code that currently works in asm_examples/serial/.
  2. ABI-ify Library/serial.S: change r17 → r24 at the entry/exit of char_write/char_read; swap any other call-saved usage for call-clobbered
  registers; add the three .global lines.
  3. Fix asm_examples/serial/main.S to pass the char in r24, read it from r24. Rebuild and re-test the asm example — that confirms the asm
  side still works.
  4. Pick a C example folder — easiest is a new examples/serial_asm/ so you don't disturb the existing C one. Copy a main.c that calls
  init_serial/char_write/char_read, plus the new serial_asm.h, plus a copy of serial.S (mirroring the C-example "duplicate files"
  convention).
  5. Local Makefile: add the %.o: %.S rule and SOURCES_S lines either to the root Makefile (cleaner, helps every future example) or as a local
  override.
  6. Build, flash, scope it. If timing's off, the only thing to tune is the period/half_period constants in the asm — none of the C-side issues
  from before are in play anymore.
  7. Once that works, the higher-level helpers (soft_string_write, soft_int16_write, etc.) can be rewritten in C against the new C-visible
  primitives — and they automatically inherit the cycle-deterministic timing.

  Which piece do you want to start on — the ABI rework on serial.S, or the Makefile changes to teach the C build about .S files?
