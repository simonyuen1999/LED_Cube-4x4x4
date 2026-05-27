# ICStation 4×4×4 LED Cube — Arduino Library

An Arduino library for driving a 4×4×4 monochrome LED cube from ICStation.com.
Controls 64 LEDs using two cascaded 74HC595 shift registers (16 outputs) and
4 direct digital pins for layer selection.

## Hardware Wiring

| Arduino Pin | Connected To         | Function                    |
|-------------|----------------------|-----------------------------|
| 0           | 74HC595 SH_CP        | Shift register clock        |
| 1           | 74HC595 ST_CP        | Shift register latch        |
| 2           | 74HC595 ~E           | Output enable (active low)  |
| 3           | 74HC595 DS           | Shift register data         |
| 4           | Layer 0 cathode      | LED_Layer0                  |
| 5           | Layer 1 cathode      | LED_Layer1                  |
| 6           | Layer 2 cathode      | LED_Layer2                  |
| 7           | Layer 3 cathode      | LED_Layer3                  |

Two 74HC595 shift registers are cascaded to produce 16 parallel outputs,
each driving one column anode wire. The four layer pins control NPN
transistors that sink current through each layer's common cathode.

**Total:** 8 Arduino pins control 64 LEDs via POV multiplexing.

## How It Works

### Matrix topology

The cube is wired as a **16-column × 4-layer** matrix. Each column has 4 LEDs
stacked vertically (one per layer). At any instant, one layer is active and the
16 column outputs determine which LEDs in that layer light up. Scanning all
4 layers fast enough (every ~200–400 µs) creates a flicker-free 3D image
through persistence of vision (POV).

### HC595 bit layout

The 16 HC595 outputs are mapped as:

```
Bit 15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0
Col grp   0     |     1      |     2      |     3
Layer   0  1 2 3|  0  1  2  3|  0  1  2  3|  0  1  2  3
```

Each column group corresponds to one of the 4 physical column positions in
the 4×4 grid. Within a group, bits 0-3 control layers 0-3 for that column.

### Frame format

A frame is 16 bytes (`_frameBuffer[16]`). Byte `j` controls column group
`j % 4`. Bits 0-3 of each byte control layers 0-3.

For example, to light the LED at column group 2, layer 1:
`_frameBuffer[2]` has bit 1 set.

### Display pipeline

```
loop()
  └─ run_example()
       ├─ If _bootStep > 0 (boot sequence):
       │     every ICStation_delay_MAX iterations:
       │       render digit (9→1) scrolling down 1 layer step
       │       _bootStep--
       │
       ├─ Else (normal patterns):
       │     every ICStation_delay_MAX iterations:
       │       generate next 16-byte frame via current pattern
       │       function into _frameBuffer
       │
       └─ Every iteration: my_display(_frameBuffer)
            └─ Show one layer:
                 1. Build 16-bit HC595 word for _scanLayer only
                 2. Latch shift registers (write_74HC595)
                 3. Turn off all layers (HIGH)
                 4. Turn on _scanLayer (LOW)
                 5. Advance _scanLayer (0→1→2→3→0→...)
```

## Pattern Generators

Animation patterns are generated programmatically by 10 functions in
`ICStation_Light_cube.cpp`. Each function fills the 16-byte `_frameBuffer`
based on a frame counter. A `patterns[]` schedule table defines each
function's frame length and cycles through them automatically.

### Available patterns

| # | Name | Frames | Behavior |
|---|------|--------|----------|
| 1 | Staircase | 32 | Columns fill diagonally — stair-step wave across the cube |
| 2 | Layer Sweep | 8 | One full layer plane at a time, sweeping up then down |
| 3 | Column Sweep | 16 | Full-height columns sweep left to right |
| 4 | Checkerboard | 4 | Alternating LEDs toggle every frame |
| 5 | Sparkle | 64 | Pseudo-random LEDs blink (LCG, no analogRead needed) |
| 6 | Rain | 32 | Drops fall from top layer to bottom, staggered |
| 7 | Expanding Box | 8 | Lit region expands from center then contracts |
| 8 | Snake | 64 | Single LED traces through all 16 columns × 4 layers |
| 9 | Bar Graph | 16 | Rotating bar heights per column |
| 10 | Blink | 4 | All LEDs on/off

## Boot Sequence

On power-up, the cube runs a countdown animation before cycling through
the normal patterns. Digits **9 through 1** appear as 4×4 pixel images at
the top layer and scroll downward through the cube layer by layer. Each
digit takes 4 scroll steps (fully visible → almost off-screen), and the
entire sequence lasts ~36 seconds at `ICStation_delay_MAX = 10000`.

The boot state is tracked by `_bootStep`, which starts at 36 and counts
down to 0. During the boot phase, `run_example()` calls `renderBootDigit()`
instead of the pattern generator. Once `_bootStep` reaches 0, normal pattern
cycling begins automatically.

## Fixes & Enhancements Applied

### 1. PROGMEM for pattern data

The `PatternTable[]` was originally stored in RAM (~1.8 KB on an Arduino Uno
with only 2 KB SRAM). With PROGMEM + `pgm_read_byte()`, the table lives in
Flash (32 KB) and leaves SRAM for the call stack.

**Files changed:**
- `ICStation_Light_cube.h`: added `const` to `PatternTable` declaration
- `ICStation_Light_cube.cpp`: added `PROGMEM` attribute,
  `#include <avr/pgmspace.h>`, `pgm_read_byte()` in `run_example()`

### 2. Layer POV multiplexing

The original code kept all 4 layers permanently ON and only set the 74HC595
column outputs. This meant the cube displayed a flat 2D (4×4) pattern — all
layers showed the same content.

The rewrite implements proper time-division multiplexing: one layer per
`loop()` iteration, cycling continuously.

**Files changed:**
- `ICStation_Light_cube.h`: renamed `LED_Pin16-19` → `LED_Layer0-3`;
  added `_frameBuffer[16]` (persistent frame storage) and `_scanLayer`
- `ICStation_Light_cube.cpp`: rewrote `my_display()` and `run_example()`
  for per-layer multiplexing; added cases 18-19 to `dight_write_LED_pin()`

### 3. dight_write_LED_pin() completed

The switch statement only handled values 0-17. Values 18-19 were silent
no-ops. Now cases 18 and 19 control `LED_Layer2` and `LED_Layer3`.

### 4. Boot sequence countdown

A startup countdown (9 → 1) was added. Each digit is rendered as a 4×4
pixel image across all 4 cube layers, then scrolls downward through the
cube before the next digit appears. Uses `digitFont[10][4]` and
`renderBootDigit()` for the scrolling effect.

**Files changed:**
- `ICStation_Light_cube.h`: added `_bootStep` member
- `ICStation_Light_cube.cpp`: added `digitFont[][]` array, `renderBootDigit()`
  function, and boot-phase logic in `run_example()`

### 5. Programmatic pattern generation

The original `PatternTable[]` was a ~1.8 KB hardcoded array of ~108 frames
stored in Flash via PROGMEM. It was replaced with 10 algorithmically
generated pattern functions. Each function fills the 16-byte `_frameBuffer`
using a frame counter, creating dynamic, memory-free animations. A pattern
schedule table cycles through them automatically.

**Files changed:**
- `ICStation_Light_cube.h`: removed `PatternTable`, `PatternIdx`, `PatternMax`
  declarations; added `_patternFrame`
- `ICStation_Light_cube.cpp`: removed the entire PatternTable array and all
  PROGMEM/pgm_read_byte references; added 10 pattern generator functions
  (`genStaircase`, `genLayerSweep`, `genColumnSweep`, `genCheckerboard`,
  `genSparkle`, `genRain`, `genExpandingBox`, `genSnake`, `genBarGraph`,
  `genBlink`); added a `patterns[]` schedule table; rewrote `run_example()`
  to call pattern functions instead of reading from the table

## API Reference

### `ICStation_Light_cube()`
Constructor. Initializes all pins as OUTPUT. Sets all layers OFF (HIGH).

### `void my_display(unsigned char *planeBuf)`
Display one layer of a 16-byte frame buffer. Which layer is determined by
`_scanLayer`, which advances each call. Called from `run_example()`.

### `void dight_write_LED_pin(int value, int charge)`
Low-level pin control. Values 0-15 manipulate 74HC595 bits. Values 16-19
control layer pins. Always calls `write_74HC595()`.

### `void write_74HC595()`
Shift out `HC_595_Temp` (16 bits, LSB-first) to the cascaded 74HC595s
and latch the outputs.

### `void run_example()`
Main animation driver. Call from `loop()`. Cycles through 10 pattern
generators, calling `my_display()` every iteration for continuous
POV multiplexing.

## Example

See `examples/Specialeffects/Specialeffects.ino`:

```cpp
#include <ICStation_Light_cube.h>

ICStation_Light_cube My_Light_cube;

void setup() {}

void loop() {
  My_Light_cube.run_example();
}
```
