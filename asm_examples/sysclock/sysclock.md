# Claude analysis of sysclock timing loop

* Count = 30,000 => 100ms
* OCR0A = 0x95 or 149
* Prescalar = /8

Result = 0x66 or 102

**Update:** Reduced ISR overhead by 10bytes, by using r2 as a constant ISR temp register.
New Result = 0x65 or 101

Remaining analysis is the same.

## Analysis

Confirmed — 0x66 (102) is the right number. Here's why it's 102 and not the "obvious" 100.

  Tick period (how often r24:r25 increments)

  ┌─────────────────────────┬─────────────────────────────┐
  │          Item           │            Value            │
  ├─────────────────────────┼─────────────────────────────┤
  │ OCR0A                   │ 0x95 = 149                  │
  ├─────────────────────────┼─────────────────────────────┤
  │ CTC TOP period          │ 1 + 149 = 150 timer counts  │
  ├─────────────────────────┼─────────────────────────────┤
  │ Prescaler (CS01)        │ ÷8                          │
  ├─────────────────────────┼─────────────────────────────┤
  │ CPU cycles per tick     │ 150 × 8 = 1200              │
  ├─────────────────────────┼─────────────────────────────┤
  │ Time per tick @ 1.2 MHz │ 1200 / 1,200,000 = 1.000 ms │
  └─────────────────────────┴─────────────────────────────┘

  Delay loop — instruction cycles

  ┌──────────────────────────────────────┬───────────┐
  │             Instruction              │  Cycles   │
  ├──────────────────────────────────────┼───────────┤
  │ sbiw r26,1                           │ 2         │
  ├──────────────────────────────────────┼───────────┤
  │ brne taken                           │ 2         │
  ├──────────────────────────────────────┼───────────┤
  │ → per normal iteration               │ 4         │
  ├──────────────────────────────────────┼───────────┤
  │ Final iteration (brne falls through) │ 2 + 1 = 3 │
  ├──────────────────────────────────────┼───────────┤
  │ Total: 29,999 × 4 + 3                │ 119,999   │
  └──────────────────────────────────────┴───────────┘

  Naïvely: 119,999 / 1200 = 99.99 ≈ 100 ticks. So where do the extra ~2 come from?

  ISR cost per invocation

  The interrupt doesn't change the timer, but it does steal CPU cycles from the loop — so the loop's wall-clock time is longer than its instruction count.

  ┌──────────────────────────────────────┬────────┐
  │                 Step                 │ Cycles │
  ├──────────────────────────────────────┼────────┤
  │ Interrupt latency (PC push + vector) │ 4      │
  ├──────────────────────────────────────┼────────┤
  │ rjmp TIM0_COMPA_handler              │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ push r16                             │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ in r16, SREG                         │ 1      │
  ├──────────────────────────────────────┼────────┤
  │ push r16                             │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ adiw r24,1                           │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ pop r16                              │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ out SREG, r16                        │ 1      │
  ├──────────────────────────────────────┼────────┤
  │ pop r16                              │ 2      │
  ├──────────────────────────────────────┼────────┤
  │ reti                                 │ 4      │
  ├──────────────────────────────────────┼────────┤
  │ Total per ISR                        │ 22     │
  └──────────────────────────────────────┴────────┘

  Putting it together

  Wall-clock time of the loop = loop instructions + every ISR that fires during it:

  T_wall = 119,999 + 22 · n      where  n = T_wall / 1200

  Solving the fixed point:

  T_wall = 119,999 / (1 − 22/1200) = 119,999 × 1200/1178 ≈ 122,240 cycles
  n      = 122,240 / 1200 ≈ 101.9 ticks

  ┌──────────────────────────┬──────────┬───────────────┐
  │                          │  Cycles  │     Ticks     │
  ├──────────────────────────┼──────────┼───────────────┤
  │ Delay loop instructions  │ 119,999  │ 100.0         │
  ├──────────────────────────┼──────────┼───────────────┤
  │ ISR overhead (~102 × 22) │ ~2,240   │ ~1.9          │
  ├──────────────────────────┼──────────┼───────────────┤
  │ Total measured window    │ ~122,240 │ ≈ 101.9 → 102 │
  └──────────────────────────┴──────────┴───────────────┘

  The count of compare-matches inside a 122,240-cycle window is 101–102 depending on phase; since the loop starts deterministically relative to the timer, you get a consistent 102 = 0x66. ✓

  Note r7 (delta high byte) prints as 0x00 — correct, since 102 fits in one byte. And the inline comment ; 299@1.2MHZ => 1ms delay is now stale: 
  COUNTER = 30000 is deliberately a ~100 ms window so the delta is visible. If you ever want delta ≈ exactly 100, you'd need to shave the loop to compensate for the 22-cycle-per-tick ISR tax.
