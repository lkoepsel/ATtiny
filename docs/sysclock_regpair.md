❯ One question i had regarding the ABI changes is that I want to use a register pair in @asm_examples/sysclock/main.S which will remain constant
  and be somewhat safe to use anywhere. What would be the best way to handle this, given the constraints of the C ABI, conventions and so on. I
  had beeen using r25:r24 and clearly this won't work.
  ⎿  Read asm_examples/sysclock/main.S (152 lines)

⏺ Good question — and r25:r24 is the worst possible choice under the ABI: it's both the first-argument register and the byte-return register.
  Every C call writes to it. So your global counter would be trashed by rcall char_write(byte) (which loads the byte into r24) and again on every
  function return.

  The framework — what makes a register pair "safe persistent"

  For a value that lives across function calls and ISRs, you need a pair that satisfies four constraints:

  1. Not in the argument/return registers (r25:r24, r23:r22, r21:r20, r19:r18) — these are touched on every call.
  2. Call-saved, so the contract says no function may mutate it. Range: r2–r17, r28, r29.
  3. Not already claimed elsewhere in your codebase (r2 is your ISR STATUS save; r4:r5 and r6:r7 are your snapshot pairs in main_loop; r16 is your
   temp; r17 is the softserial char register).
  4. Even-numbered low byte, so movw works on it (movw operates on any even/odd pair, so r8:r9, r10:r11, … all qualify).

  The candidates that pass all four: r8:r9, r10:r11, r12:r13, r14:r15 (and the Y pair r28:r29 if you give up the C frame pointer — usually not
  worth it on this chip).

  Recommended: r8:r9

  Lowest unused pair, no overlap with anything you're already doing, leaves r14:r15 free for future ISR scratch if you ever need a 16-bit ISR
  temp. Pick the lowest free pair as a habit so future expansion has somewhere to go.

  What this looks like in sysclock/main.S

  Two changes plus a third caveat:
```asm
   ; ---------- Reserved registers ----------
   ; r2        ISR scratch (STATUS save) — do NOT use elsewhere
  -; r25:r24   global 16-bit ISR ticks counter — do NOT use elsewhere
  +; r9:r8     global 16-bit ISR ticks counter — do NOT use elsewhere

   init_sysclock_1k:
  -    eor     r24, r24        ; clear counter low byte
  -    eor     r25, r25        ; clear counter high byte
  +    eor     r8, r8          ; clear counter low byte
  +    eor     r9, r9          ; clear counter high byte

   TIM0_COMPA_handler:
       in      r2, STATUS
  -    adiw    r24, 1
  +    inc     r8              ; r8++
  +    brne    1f              ; no carry → done
  +    inc     r9              ; carry → bump high byte
  +1:
       out     STATUS, r2
       reti

       ; get clock start
  -    movw    r4, r24         ; snapshot r25:r24 → r5:r4
  +    movw    r4, r8          ; snapshot r9:r8 → r5:r4
       ...
       ; get clock end
  -    movw    r6, r24
  +    movw    r6, r8
```
  The adiw caveat

  adiw (Add Immediate to Word) only works on r25:r24, r27:r26, r29:r28, r31:r30 — the four "upper" pointer pairs. r8:r9 isn't one of them, so the
  ISR loses its single-instruction increment and pays +1 cycle on average (inc=1, brne-not-taken=1 → 2 cycles in the common case; the carry path
  runs once per 256 ticks). New ISR cost: 15 cycles instead of 14. Update sysclock.md accordingly when you make the change.

  If keeping adiw matters more than the four constraints above, the only other choice is the X register pair (r27:r26) — call-clobbered,
  adiw-compatible, but you'd have to either save/restore it in the ISR (defeats the point) or reserve it globally with -ffixed-r26 -ffixed-r27 on
  the C side (costs you one pointer register in C, which on this chip rarely matters). Either trade-off is defensible; I'd take the +1 ISR cycle
  and keep the call-saved pair.

  The atomicity bonus you get for free

  Because the ISR is the only writer and main is the only reader, and movw is a single 1-cycle instruction, your snapshot reads (movw r4, r8) are
  atomic with respect to the ISR. The CPU cannot service an interrupt mid-movw, so the reader either sees the pair before the ISR's update or
  after — never a torn half-updated value. No cli / sei needed.

  If you ever needed to read just the low byte alone (mov r4, r8) that's still atomic (1 instruction), but reading the bytes individually (mov r4,
   r8; mov r5, r9 — two instructions) would be torn-readable: the ISR could fire between them and you'd see byte 0 from the old value and byte 1
  from the new. Always use movw for these snapshots.

  When C enters the picture

  Today, r9:r8 "stays put" purely by developer discipline — you've documented it as reserved and you've personally promised not to touch it
  elsewhere. The assembler/linker won't enforce that. In pure asm that's fine.

  Once you start mixing in C (per docs/asm_from_c.md), the C compiler doesn't know about your reservation and will happily allocate r8 and r9 for
  its own variables. You need to tell it not to. Two pieces:

  1. Compiler flags — add to the C Makefile's CFLAGS:

  CFLAGS += -ffixed-r8 -ffixed-r9

  -ffixed-rN removes rN from the compiler's allocation pool entirely. For the whole compilation, the C side will pretend r8 and r9 don't exist.

  2. C-side declaration — in your C header:

  register uint16_t ticks asm("r8");

  This binds the C variable ticks to the physical register pair r9:r8 (gcc takes the named low byte and the next register as the upper byte for a
  16-bit type). Now reading or writing ticks from C compiles directly to register operations on r8/r9, sharing the same physical pair the asm
  uses. No address-of, no volatile needed for the access — but you do still need to remember the atomicity rule (gcc may break a 16-bit read into
  two byte moves at low optimization levels, so wrap multi-byte access to a shared global with cli/sei if you're not relying on movw).

  Other approaches and why I didn't recommend them

  - Memory-resident (in .bss). Most ABI-pure: no register reservation, the ISR does lds/sts on a 2-byte variable. Cost: the ISR balloons (lds +
  lds + 16-bit inc with carry + sts + sts + a scratch pair save/restore, since adiw only takes register pairs). Roughly 3× the ISR cycles. Not
  worth it when one call-saved pair solves it cleanly.
  - Y register (r29:r28) with -ffixed-. Adiw-compatible, but C uses Y as the frame pointer. Locking it forces gcc to spill frames awkwardly.
  Disruptive for what is otherwise a single-byte change in flag count.
  - Call-clobbered pair with ISR save/restore. Defeats the purpose — every ISR pays a stack round-trip to "hold" a value that was never threatened
   in the first place. Use call-saved.

  **Summary:**

Reserve r8:r9 (or any unused call-saved pair). Document the reservation in your asm header comment. Use movw for atomic snapshots from the ISR-written pair. Pay 1 extra ISR cycle for losing adiw. When you add C, gate it behind -ffixed-r8 -ffixed-r9 and declare register uint16_t ticks asm("r8"); so both sides share the same physical pair.
