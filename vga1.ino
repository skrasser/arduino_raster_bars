// VGA output on a 16MHz Arduino Uno for the VESA 640x480@75Hz video mode

// Pin mappings for VGA; colors are on portd, horizontal sync is on portb,
// vertical sync is on portc; pins connected over 470 Ohm resistor each
#define H_SYNC 13 // PB0
#define V_SYNC A5 // PC0
// PDX
#define RED0 0    // PD0
#define GRN0 1    // PD1
#define GRN1 2
#define GRN2 3
#define BLU0 4
#define BLU1 5
#define BLU2 6
#define BKGR 7

#define BGRED0 8
#define BGRED1 9
#define BGRED2 10
#define BGRED3 11
#define BGRED4 12
#define BGGRN0 A0
#define BGGRN1 A1
#define BGGRN2 A2
#define BGGRN3 A3
#define BGGRN4 A4

// 106 pixels width for the line buffer, each pixel in the buffer corresponds
// to 6 pixels in the VGA output.
#define LINEWIDTH 106

volatile
uint8_t linebuf[LINEWIDTH] __attribute__((aligned(256))); // declare as volatile since asm code will change this

extern const
int8_t sinetab[128] __attribute__((aligned(256))) = {
  0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 59, 64, 70, 75, 80, 84,
  89, 93, 97, 101, 105, 108, 111, 114, 116, 119, 120, 122, 123, 125, 125, 126,
  126, 126, 125, 125, 123, 122, 120, 119, 116, 114, 111, 108, 105, 101, 97, 93,
  89, 84, 80, 75, 70, 64, 59, 54, 48, 42, 36, 30, 24, 18, 12, 6,
  0, -6, -12, -18, -24, -30, -36, -42, -48, -54, -59, -64, -70, -75, -80, -84,
  -89, -93, -97, -101, -105, -108, -111, -114, -116, -119, -120, -122, -123, -125, -125, -126,
  -126, -126, -125, -125, -123, -122, -120, -119, -116, -114, -111, -108, -105, -101, -97, -93,
  -89, -84, -80, -75, -70, -64, -59, -54, -48, -42, -36, -30, -24, -18, -12, -6
};

uint8_t lumtab_grn[128] __attribute__((aligned(256)));
uint8_t lumtab_blu[128] __attribute__((aligned(256)));
uint8_t gradtab[256] __attribute__((aligned(256)));

void calc_lumtab(void) {
  size_t i;
  unsigned int val;
  for(i = 0; i < sizeof(sinetab); ++i) {
    val = 128u + sinetab[i];
    val = (val >> 5) & 7;
    // Make sure val is at least 1, then shift up for green output
    lumtab_grn[i] = (val ? val : 1) << 1;
    // For blue, invert luminance, shift up to blue output
    val = 7 - val;
    lumtab_blu[i] = (val ? val : 1) << 4;
  }
}

void calc_gradtab(void) {
  size_t i;
  for(i = 0; i < 32; ++i) {
    gradtab[i] = i;
    gradtab[63 - i] = i;
  }
  for(i = 64; i < sizeof(gradtab); ++i) {
    gradtab[i] = 0;
  }
}

void setup() {
  calc_lumtab();
  calc_gradtab();
  pinMode(H_SYNC, OUTPUT);
  pinMode(V_SYNC, OUTPUT);
  pinMode(RED0, OUTPUT);
  pinMode(GRN0, OUTPUT);
  pinMode(GRN1, OUTPUT);
  pinMode(GRN2, OUTPUT);
  pinMode(BLU0, OUTPUT);
  pinMode(BLU1, OUTPUT);
  pinMode(BLU2, OUTPUT);
  pinMode(BKGR, OUTPUT);
  pinMode(BGRED0, OUTPUT);
  pinMode(BGRED1, OUTPUT);
  pinMode(BGRED2, OUTPUT);
  pinMode(BGRED3, OUTPUT);
  pinMode(BGRED4, OUTPUT);
  pinMode(BGGRN0, OUTPUT);
  pinMode(BGGRN1, OUTPUT);
  pinMode(BGGRN2, OUTPUT);
  pinMode(BGGRN3, OUTPUT);
  pinMode(BGGRN4, OUTPUT);
  cli();
}

void loop() {
  __asm__ __volatile__ ("jmp start");
}
