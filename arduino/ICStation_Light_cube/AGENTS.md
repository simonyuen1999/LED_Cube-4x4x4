# Project Instructions — ICStation 4×4×4 LED Cube

## Project Type

Arduino library (C++) for driving a 4×4×4 monochrome LED cube.

## Build

This is an Arduino library project — no standalone build system. Install the
`ICStation_Light_cube/` folder into your Arduino `libraries/` directory.

**Verification:** Open `examples/Specialeffects/Specialeffects.ino` in the
Arduino IDE, select an Arduino Uno board, and verify/compile.

## Key Source Files

| File | Purpose |
|------|---------|
| `ICStation_Light_cube.h` | Class declaration, pin constants, pattern state |
| `ICStation_Light_cube.cpp` | Implementation: POV multiplexing, shift register control, frame animation |
| `examples/Specialeffects/Specialeffects.ino` | Minimal example — instantiates the class and calls `run_example()` in `loop()` |
| `README.md` | Full documentation: wiring, bit layout, pipeline, fixes applied |

## Boot Sequence

On power-up, the cube runs a countdown animation (9 → 1) that runs
for 36 frame steps before normal pattern cycling begins. Each digit
is a 4×4 pixel image rendered across all 4 cube layers. The digit
scrolls downward through the layers over 4 steps, then the next
digit appears.

## Architecture Summary

- **2× 74HC595** shift registers → 16 column anodes (LSB-first shiftOut)
- **4 digital pins** (4-7) → layer cathodes via NPN transistors. `LOW` = layer ON
- **POV multiplexing:** one layer per `loop()` iteration, cycling 0→1→2→3→0→...
- **Pattern data:** generated programmatically by 10 pattern functions in
  `ICStation_Light_cube.cpp`. Each function fills the 16-byte `_frameBuffer`
  based on a frame counter. A `patterns[]` schedule table defines each
  function's frame length and cycles through them automatically.

## Fixes & Enhancements Applied (history for context)

1. **PROGMEM** — Moved `PatternTable[]` from SRAM to Flash to free ~1.8 KB
2. **Layer multiplexing** — Rewrote `my_display()` and `run_example()` to
   cycle layers one-at-a-time instead of keeping all 4 layers permanently ON.
   This restored proper 3D display from the per-layer pattern data.
3. **Renamed pins** — `LED_Pin16-19` → `LED_Layer0-3` for clarity
4. **Completed switch cases** — Added cases 18-19 to `dight_write_LED_pin()`
5. **Added state** — `_frameBuffer[16]` for persistent frame storage,
   `_scanLayer` for multiplexing position
6. **Boot sequence** — Added countdown (9 → 1) that runs on power-up
   before normal patterns start. Digits scroll downward through the layers.
7. **Programmatic patterns** — Replaced the hardcoded ~1.8 KB PatternTable
   array with 10 algorithmically generated patterns (staircase, layer sweep,
   column sweep, checkerboard, sparkle, rain, expanding box, snake,
   bar graph, blink). Patterns cycle automatically via a schedule table.

## Important Notes for Future Work

- The `PlanePin[4]` array in the class is declared and defined but **unused**
  anywhere — it's leftover from the original code. Consider removing it.
- The `dight_write_LED_pin()` function has a guard `if(value > 19) return;`
  that protects values up to 19, but `_BV(a)` only works correctly for `a`
  up to 15 (it shifts a 16-bit `unsigned int`). Values 0-15 are the
  intended HC595 bit range.
- The `HC_595_E` pin is toggled during `write_74HC595()` to disable output
  while shifting — this prevents intermediate bit states from ghosting.
- `ICStation_delay_MAX = 10000` controls frame rate. Each `loop()` iteration
  is roughly 50-100 µs, so each frame lasts ~0.5-1 second. Adjust for
  faster/slower animation.
- No `delayMicroseconds()` calls are used in the multiplexing loop — the
  natural `loop()` rate provides sufficient POV persistence for most LEDs.
  If brightness is too low, add a `delayMicroseconds(2000)` in `my_display()`
  after turning on the layer.
