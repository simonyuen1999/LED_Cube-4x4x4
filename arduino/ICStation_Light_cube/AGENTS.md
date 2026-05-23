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
| `ICStation_Light_cube.h` | Class declaration, pin constants, PROGMEM pattern table |
| `ICStation_Light_cube.cpp` | Implementation: POV multiplexing, shift register control, frame animation |
| `examples/Specialeffects/Specialeffects.ino` | Minimal example — instantiates the class and calls `run_example()` in `loop()` |
| `README.md` | Full documentation: wiring, bit layout, pipeline, fixes applied |

## Architecture Summary

- **2× 74HC595** shift registers → 16 column anodes (LSB-first shiftOut)
- **4 digital pins** (4-7) → layer cathodes via NPN transistors. `LOW` = layer ON
- **POV multiplexing:** one layer per `loop()` iteration, cycling 0→1→2→3→0→...
- **Pattern data:** stored in Flash via PROGMEM (~1.8 KB), loaded into a
  persistent 16-byte `_frameBuffer` every `ICStation_delay_MAX` iterations

## Fixes & Enhancements Applied (history for context)

1. **PROGMEM** — Moved `PatternTable[]` from SRAM to Flash to free ~1.8 KB
2. **Layer multiplexing** — Rewrote `my_display()` and `run_example()` to
   cycle layers one-at-a-time instead of keeping all 4 layers permanently ON.
   This restored proper 3D display from the per-layer pattern data.
3. **Renamed pins** — `LED_Pin16-19` → `LED_Layer0-3` for clarity
4. **Completed switch cases** — Added cases 18-19 to `dight_write_LED_pin()`
5. **Added state** — `_frameBuffer[16]` for persistent frame storage,
   `_scanLayer` for multiplexing position

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
