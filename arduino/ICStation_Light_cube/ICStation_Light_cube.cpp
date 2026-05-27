/************************************************************************************
  All materials are provided by www.ICStation.com.
Should you have any unclear or need any other related material, please feel free to 
contact us via http://www.icstation.com/contact_us.php.
www.ICStation.com
   ICStation is based in Shenzhen, China, a city which is rich in electronics. 
With such manufacture power, we not only provide worldwide with all kinds of 
IC products, such as electronic devices and components, development modules, 
development boards, consumptive materials and so on, but also we are taking 
part in the designing and developing digital and analog circuit design which
 based on  microcontroller platforms.
**************************************************************************************/
#include "ICStation_Light_cube.h"

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
int ICStation_Light_cube::LEDPin[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
int ICStation_Light_cube::PlanePin[4] = {16, 17, 18, 19};
unsigned int ICStation_Light_cube::HC_595_Temp = 0;
int ICStation_Light_cube::clockPin = 0;    // SH_CP of 74HC595
int ICStation_Light_cube::latchPin = 1;    // ST_CP of 74HC595
int ICStation_Light_cube::dataPin = 3;     // DS of 74HC595
int ICStation_Light_cube::HC_595_E = 2;    // 74HC595 output enable
int ICStation_Light_cube::LED_Layer0 = 4;
int ICStation_Light_cube::LED_Layer1 = 5;
int ICStation_Light_cube::LED_Layer2 = 6;
int ICStation_Light_cube::LED_Layer3 = 7;

#define ICStation_delay_MAX 10000
int ICStation_Light_cube::ICStation_delay = ICStation_delay_MAX;

int ICStation_Light_cube::_patternFrame = 0;
unsigned char ICStation_Light_cube::_frameBuffer[16] = {0};
int ICStation_Light_cube::_scanLayer = 0;
int ICStation_Light_cube::_bootStep = 36;  // 9 digits × 4 layers

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
ICStation_Light_cube::ICStation_Light_cube()
{
  pinMode( latchPin, OUTPUT );
  pinMode( clockPin, OUTPUT );
  pinMode( dataPin, OUTPUT );
  pinMode( HC_595_E, OUTPUT );

  pinMode( LED_Layer0, OUTPUT );
  pinMode( LED_Layer1, OUTPUT );
  pinMode( LED_Layer2, OUTPUT );
  pinMode( LED_Layer3, OUTPUT );

  digitalWrite(LED_Layer0, HIGH);   // HIGH = layer OFF (common-cathode)
  digitalWrite(LED_Layer1, HIGH);
  digitalWrite(LED_Layer2, HIGH);
  digitalWrite(LED_Layer3, HIGH);
}

ICStation_Light_cube::~ICStation_Light_cube()
{
}

// ---------------------------------------------------------------------------
// Display -- one layer per call, POV multiplexed
// ---------------------------------------------------------------------------
void ICStation_Light_cube::my_display(unsigned char *planeBuf)
{
  int layer = _scanLayer;

  // Build the 16-bit HC595 word for this layer only
  HC_595_Temp = 0;
  for(int col = 0; col < 16; col++)
  {
    if(planeBuf[col] >> layer & B0001)
    {
      HC_595_Temp |= _BV(15 - (4 * (col % 4) + layer));
    }
  }
  write_74HC595();

  // Turn off all layers, then turn on the current one
  digitalWrite(LED_Layer0, HIGH);
  digitalWrite(LED_Layer1, HIGH);
  digitalWrite(LED_Layer2, HIGH);
  digitalWrite(LED_Layer3, HIGH);

  switch(layer) {
    case 0: digitalWrite(LED_Layer0, LOW); break;
    case 1: digitalWrite(LED_Layer1, LOW); break;
    case 2: digitalWrite(LED_Layer2, LOW); break;
    case 3: digitalWrite(LED_Layer3, LOW); break;
  }

  // Next call shows the next layer
  _scanLayer = (layer + 1) & 3;
}

// ---------------------------------------------------------------------------
// Shift register control
// ---------------------------------------------------------------------------
void ICStation_Light_cube::write_74HC595(void)
{
  digitalWrite(latchPin, LOW);
  digitalWrite(HC_595_E, HIGH);
  shiftOut(dataPin, clockPin, LSBFIRST, HC_595_Temp);
  shiftOut(dataPin, clockPin, LSBFIRST, (HC_595_Temp >> 8));
  digitalWrite(latchPin, HIGH);
  digitalWrite(HC_595_E, LOW);
}

// ---------------------------------------------------------------------------
// Low-level pin write -- values 0-15 = HC595 bits, 16-19 = layer pins
// ---------------------------------------------------------------------------
void ICStation_Light_cube::dight_write_LED_pin(int value, int charge)
{
  if(value > 19) return;
  if(charge != LOW && charge != HIGH) return;

  if(charge == LOW)
  {
    switch (value)
    {
      case 0:  HC_595_Temp &=~_BV(15); break;
      case 1:  HC_595_Temp &=~_BV(14); break;
      case 2:  HC_595_Temp &=~_BV(13); break;
      case 3:  HC_595_Temp &=~_BV(12); break;
      case 4:  HC_595_Temp &=~_BV(11); break;
      case 5:  HC_595_Temp &=~_BV(10); break;
      case 6:  HC_595_Temp &=~_BV(9); break;
      case 7:  HC_595_Temp &=~_BV(8); break;
      case 8:  HC_595_Temp &=~_BV(7); break;
      case 9:  HC_595_Temp &=~_BV(6); break;
      case 10: HC_595_Temp &=~_BV(5); break;
      case 11: HC_595_Temp &=~_BV(4); break;
      case 12: HC_595_Temp &=~_BV(3); break;
      case 13: HC_595_Temp &=~_BV(2); break;
      case 14: HC_595_Temp &=~_BV(1); break;
      case 15: HC_595_Temp &=~_BV(0); break;
      case 16: digitalWrite(LED_Layer0, LOW); break;
      case 17: digitalWrite(LED_Layer1, LOW); break;
      case 18: digitalWrite(LED_Layer2, LOW); break;
      case 19: digitalWrite(LED_Layer3, LOW); break;
    }
  }
  else
  {
    switch (value)
    {
      case 0:  HC_595_Temp |=_BV(15); break;
      case 1:  HC_595_Temp |=_BV(14); break;
      case 2:  HC_595_Temp |=_BV(13); break;
      case 3:  HC_595_Temp |=_BV(12); break;
      case 4:  HC_595_Temp |=_BV(11); break;
      case 5:  HC_595_Temp |=_BV(10); break;
      case 6:  HC_595_Temp |=_BV(9); break;
      case 7:  HC_595_Temp |=_BV(8); break;
      case 8:  HC_595_Temp |=_BV(7); break;
      case 9:  HC_595_Temp |=_BV(6); break;
      case 10: HC_595_Temp |=_BV(5); break;
      case 11: HC_595_Temp |=_BV(4); break;
      case 12: HC_595_Temp |=_BV(3); break;
      case 13: HC_595_Temp |=_BV(2); break;
      case 14: HC_595_Temp |=_BV(1); break;
      case 15: HC_595_Temp |=_BV(0); break;
      case 16: digitalWrite(LED_Layer0, HIGH); break;
      case 17: digitalWrite(LED_Layer1, HIGH); break;
      case 18: digitalWrite(LED_Layer2, HIGH); break;
      case 19: digitalWrite(LED_Layer3, HIGH); break;
    }
  }
  write_74HC595();
}

// ===========================================================================
// Pattern Generator Functions
//
// Each function fills a 16-byte frame buffer.  Byte i controls column group
// (i % 4), bits 0-3 control layers 0-3 for that group.  The frame argument
// is the 0-based frame index within the pattern run.
// ===========================================================================

// ---- 1. Staircase Fill --------------------------------------------------
// Columns fill diagonally -- each successive column has one more layer lit,
// creating a stair-step wave.
static void genStaircase(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++)
  {
    buf[c] = 0;
    int h = frame - c;
    if(h > 3)  h = 3;
    if(h >= 0)
      for(int b = 0; b <= h && b < 4; b++)
        buf[c] |= (1 << b);
  }
}

// ---- 2. Layer Sweep -----------------------------------------------------
// One full layer plane lights up at a time, sweeping up then down.
static void genLayerSweep(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++) buf[c] = 0;
  int layer = frame % 8;
  if(layer >= 4) layer = 7 - layer;  // 0,1,2,3,2,1,0
  for(int c = 0; c < 16; c++) buf[c] |= (1 << layer);
}

// ---- 3. Column Sweep ----------------------------------------------------
// Full-height columns sweep left to right across the 4 column groups.
static void genColumnSweep(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++) buf[c] = 0;
  int group = frame % 4;
  for(int c = 0; c < 16; c++)
    if((c % 4) == group) buf[c] = 0x0F;
}

// ---- 4. Checkerboard ----------------------------------------------------
// Alternating LEDs toggle each frame.
static void genCheckerboard(unsigned char *buf, int frame)
{
  int phase = frame & 1;
  for(int c = 0; c < 16; c++)
  {
    int x = c % 4;
    int y = c / 4;
    buf[c] = ((x + y + phase) & 1) ? 0x0F : 0x00;
  }
}

// ---- 5. Random Sparkle --------------------------------------------------
// Pseudo-random LEDs blink on and off each frame (LCG, no analogRead).
static void genSparkle(unsigned char *buf, int frame)
{
  unsigned int r = frame * 5 + 12345;
  for(int c = 0; c < 16; c++)
  {
    buf[c] = 0;
    for(int b = 0; b < 4; b++)
    {
      r = r * 1103515245 + 12345;
      if((r >> 8) & 1) buf[c] |= (1 << b);
    }
  }
}

// ---- 6. Rain -----------------------------------------------------------
// Drops appear at the top (layer 3) and fall one layer per frame.
static void genRain(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++) buf[c] = 0;
  for(int d = 0; d < 4; d++)
  {
    int col = ((frame + d * 4) / 4) % 16;
    int z   = 3 - (frame + d * 4) % 4;
    buf[col] |= (1 << z);
  }
}

// ---- 7. Expanding Box ---------------------------------------------------
// A lit region expands outward from the center then contracts.
static void genExpandingBox(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++) buf[c] = 0;
  int radius = frame % 8;
  if(radius >= 4) radius = 7 - radius;
  for(int c = 0; c < 16; c++)
    if((c % 4) <= radius && (3 - c % 4) <= radius)
      buf[c] = 0x0F;
}

// ---- 8. Snake ----------------------------------------------------------
// A single LED traces through columns and layers.
static void genSnake(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++) buf[c] = 0;
  int col   = frame % 16;
  int layer = (frame / 16) % 4;
  buf[col]  = (1 << layer);
}

// ---- 9. Bar Graph -------------------------------------------------------
// Each column shows a "bar" height that rotates for a spinning effect.
static void genBarGraph(unsigned char *buf, int frame)
{
  for(int c = 0; c < 16; c++)
  {
    int h = (c + frame) % 5;
    buf[c] = 0;
    for(int b = 0; b < h && b < 4; b++)
      buf[c] |= (1 << b);
  }
}

// ---- 10. Blink ----------------------------------------------------------
// All LEDs on, then off.
static void genBlink(unsigned char *buf, int frame)
{
  unsigned char val = (frame & 1) ? 0x0F : 0x00;
  for(int c = 0; c < 16; c++) buf[c] = val;
}

// ---------------------------------------------------------------------------
// Boot-sequence digit font
// Each digit is a 4×4 pixel pattern.  Row 0 = top of digit.
// Bits 0-3 = column groups 0-3.
// {row0, row1, row2, row3}
// ---------------------------------------------------------------------------
static const unsigned char digitFont[10][4] = {
  { B1111, B1001, B1001, B1111 },  // 0 (unused)
  { B0010, B0010, B0010, B0110 },  // 1
  { B1110, B0001, B0010, B1111 },  // 2
  { B1110, B0010, B0010, B1110 },  // 3
  { B1010, B1111, B0010, B0010 },  // 4
  { B1111, B1000, B0111, B0011 },  // 5
  { B1111, B1000, B1111, B1011 },  // 6
  { B1110, B0010, B0010, B0010 },  // 7
  { B1111, B1001, B1001, B1111 },  // 8
  { B1111, B1110, B0010, B1110 },  // 9
};

// Render one boot step into _frameBuffer.
// For each of 4 layers, the digit's row is mapped to a cube layer
// with a downward scroll offset so the digit appears to fall.
static void renderBootDigit(unsigned char *buf, int digit, int scrollOffset)
{
  for(int l = 0; l < 16; l++) buf[l] = 0;
  for(int layer = 0; layer < 4; layer++)
  {
    // Map cube layer (0=bottom, 3=top) to digit row (0=top)
    int row = (3 - layer) + scrollOffset;
    if(row >= 0 && row < 4)
    {
      unsigned char rowData = digitFont[digit][row];
      // Set this row's column pattern on the current cube layer
      for(int g = 0; g < 4; g++)
        if(rowData & (1 << g))
          buf[g] |= (1 << layer);
    }
  }
}

// ---------------------------------------------------------------------------
// Pattern schedule table
// ---------------------------------------------------------------------------
#define NUM_PATTERNS 10
struct { void (*func)(unsigned char*, int); int length; }
const patterns[NUM_PATTERNS] = {
  { genStaircase,    32 },
  { genLayerSweep,    8 },
  { genColumnSweep,  16 },
  { genCheckerboard,  4 },
  { genSparkle,      64 },
  { genRain,         32 },
  { genExpandingBox,  8 },
  { genSnake,        64 },
  { genBarGraph,     16 },
  { genBlink,         4 },
};

// ===========================================================================
// Animation Driver
// ===========================================================================
void ICStation_Light_cube::run_example(void)
{
  // ---- Boot sequence: count down 9 → 1, digits falling through the cube ----
  if(_bootStep > 0)
  {
    // Each of 36 steps: the digit scrolls down by 1 layer position.
    // 4 scroll positions per digit × 9 digits (9→1) = 36 steps.
    if(ICStation_delay >= ICStation_delay_MAX)
    {
      ICStation_delay = 0;

      int digit  = (_bootStep - 1) / 4 + 1;    // 9,9,9,9, 8,8,8,8, ..., 1
      int scroll = 3 - (_bootStep - 1) % 4;    // 0,1,2,3, 0,1,2,3, ...
      renderBootDigit(_frameBuffer, digit, scroll);
      _bootStep--;
    }
    else
    {
      ICStation_delay++;
    }

    my_display(_frameBuffer);
    return;
  }

  // ---- Normal pattern animation --------------------------------------------
  static int currentPattern = 0;

  if(ICStation_delay >= ICStation_delay_MAX)
  {
    ICStation_delay = 0;

    patterns[currentPattern].func(_frameBuffer, _patternFrame);
    _patternFrame++;

    if(_patternFrame >= patterns[currentPattern].length)
    {
      _patternFrame = 0;
      currentPattern = (currentPattern + 1) % NUM_PATTERNS;
    }
  }
  else
  {
    ICStation_delay++;
  }

  my_display(_frameBuffer);
}