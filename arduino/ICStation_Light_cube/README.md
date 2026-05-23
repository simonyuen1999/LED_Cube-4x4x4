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
       ├─ Every ICStation_delay_MAX iterations:
       │     load next 16-byte frame from PROGMEM PatternTable
       │     into _frameBuffer
       │
       └─ Every iteration: my_display(_frameBuffer)
            └─ Show one layer:
                 1. Build 16-bit HC595 word for _scanLayer only
                 2. Latch shift registers (write_74HC595)
                 3. Turn off all layers (HIGH)
                 4. Turn on _scanLayer (LOW)
                 5. Advance _scanLayer (0→1→2→3→0→...)
```

## Pattern Table

`PatternTable[]` stores ~108 animation frames in **Flash** (PROGMEM),
not SRAM. Each frame is 17 bytes:

- 16 bytes of column data
- 1 byte frame-delay placeholder (skipped during playback)

The table is read-only. `PatternMax = sizeof(PatternTable)` determines
the wrap-around point.

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
Main animation driver. Call from `loop()`. Manages frame loading
from PROGMEM and calls `my_display()` every iteration for continuous
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
