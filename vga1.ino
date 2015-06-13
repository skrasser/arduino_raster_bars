// VGA output on a 16MHz Arduino Uno for the VESA 640x480@75Hz video mode

// Pin mappings for VGA; colors are on portd, horizontal sync is on portb,
// vertical sync is on portc; pins connected over 470 Ohm resistor each
#define H_SYNC 8  // PB0
#define V_SYNC A0 // PC0
// PDX
#define RED0 0    // PD0
#define GRN0 1    // PD1
#define GRN1 2
#define GRN2 3
#define BLU0 4
#define BLU1 5
#define BLU2 6

// 106 pixels width for the line buffer, each pixel in the buffer corresponds
// to 6 pixels in the VGA output.
#define LINEWIDTH 106
volatile uint8_t linebuffer[LINEWIDTH] __attribute__((aligned(256))); // declare as volatile since asm code will change this
const int8_t sinetab[128] __attribute__((aligned(256))) = {
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

#define mdbl(macro, after, before) \
  ".macro " #macro #after "\n" \
  #macro #before "\n" \
  #macro #before "\n" \
  ".endm \n"

// Delay macros
__asm__ __volatile__ (
  ".macro delay1 \n"
  "  nop \n"
  ".endm \n"

  ".macro delay2 \n"
  "  delay1 \n"
  "  delay1 \n"
  ".endm \n"

  ".macro delay4 \n"
  "  delay2 \n"
  "  delay2 \n"
  ".endm \n"

  ".macro delay8 \n"
  "  delay4 \n"
  "  delay4 \n"
  ".endm \n"
  
  ".macro delay16 \n"
  "  delay8 \n"
  "  delay8 \n"
  ".endm \n"
  
  mdbl(delay, 32, 16)
  mdbl(delay, 64, 32)
);

// Output macros
__asm__ __volatile__ (
  ".macro out1 reg,port \n"
  "  ld \\reg, X+ \n"        // 2
  "  out \\port, \\reg \n"   // 1
  ".endm \n"
  
  ".macro out2 reg,port \n"
  "  out1 \\reg, \\port \n"
  "  out1 \\reg, \\port \n"
  ".endm \n"
  
  ".macro out4 reg,port \n"
  "  out2 \\reg, \\port \n"
  "  out2 \\reg, \\port \n"
  ".endm \n"

  ".macro out8 reg,port \n"
  "  out4 \\reg, \\port \n"
  "  out4 \\reg, \\port \n"
  ".endm \n"
  
  ".macro out16 reg,port \n"
  "  out8 \\reg, \\port \n"
  "  out8 \\reg, \\port \n"
  ".endm \n"

  ".macro out32 reg,port \n"
  "  out16 \\reg, \\port \n"
  "  out16 \\reg, \\port \n"
  ".endm \n"
  
  ".macro out64 reg,port \n"
  "  out32 \\reg, \\port \n"
  "  out32 \\reg, \\port \n"
  ".endm \n"

  ".macro out106 reg,port \n"
  "  out64 \\reg, \\port \n"
  "  out32 \\reg, \\port \n"
  "  out8 \\reg, \\port \n"
  "  out2 \\reg, \\port \n"
  ".endm \n"
);

// Fast memory clear, i.e. w/o loop (memory pointed to by X)
__asm__ __volatile__ (
  ".macro clrmem1 \n"
  "  st X+, __zero_reg__ \n"        // 2
  ".endm \n"
  
  mdbl(clrmem, 2, 1)
  mdbl(clrmem, 4, 2)
  mdbl(clrmem, 8, 4)
  mdbl(clrmem, 16, 8)
  mdbl(clrmem, 32, 16)
  mdbl(clrmem, 64, 32)

  ".macro clrmem106 \n"
  "  clrmem64 \n"
  "  clrmem32 \n"
  "  clrmem8 \n"
  "  clrmem2 \n"
  ".endm \n"
 );

void setup() {
  calc_lumtab();
  pinMode(H_SYNC, OUTPUT);
  pinMode(V_SYNC, OUTPUT);
  pinMode(RED0, OUTPUT);
  pinMode(GRN0, OUTPUT);
  pinMode(GRN1, OUTPUT);
  pinMode(GRN2, OUTPUT);
  pinMode(BLU0, OUTPUT);
  pinMode(BLU1, OUTPUT);
  pinMode(BLU2, OUTPUT);
  cli();
}

void loop() {
   __asm__ __volatile__ (
   "ldi r26, lo8(%[linebuf]) \n"    // load linebuffer ptr into X
   "ldi r27, hi8(%[linebuf]) \n"
   
   // Keeping some numbers handy
   "ldi r19, 2 \n"    // ldi needs registers > r15
   "mov r2, r19 \n"   // r2 <= 2
   "ldi r19, 4 \n"
   "mov r3, r19 \n"   // r3 <= 4
   "ldi r19, 6 \n"
   "mov r4, r19 \n"   // r4 <= 6
   "ldi r19, 127 \n"
   "mov r4, r19 \n"   // r5 <= 127
   
   "ldi r24, 1 \n"
   "ldi r23, 3 \n"
   "ldi r22, 7 \n"
   
   // Other registers used:
   // r25 -- line counter
   // r28 -- delay/column counter or offset
   // r21 -- output color
   // r20 -- logical line, increment every 4 raster lines
   // r19 -- misc
   // r18 -- frame counter/time
   // r17 -- misc
   
   "ldi r18, 0 \n"
      
   "loop_frame: \n"
   // Starting with vsync output
   
   // Vsync front porch: 1 line
   ////////////////////////////
   "call black_line \n"     // 4 cycles for the call
   "call wait52 \n"         // 52 cycles total
   
   // Vsync pulse: 3 lines
   ///////////////////////
   "out %[portc], r24 \n"   // 1 cycle, unaccounted in timing
   "ldi r25, 3 \n"          // 1
   "v_sync_pulse: \n"
   "call black_line \n"     // 4
   "call wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne v_sync_pulse \n"   // 2 if taken, 1 otherwise
   "out %[portc], __zero_reg__ \n"   // 1 cycle, unaccounted in timing
  
   // Vsync back porch: 16 lines
   /////////////////////////////
   // First line: clear buffer
   "nop \n"         // 1
   "call black_line_clr_buf \n"     // 4
   "call prep_wait52 \n"
   "ldi r20, 0 \n"          // 1, set logical line back to 0
   "nop \n"                 // 1
   "nop \n"                 // 1
   // 15 more lines
   "ldi r25, 15 \n"         // 1
   "v_back_porch: \n"
   "call black_line \n"     // 4
   "call prep_wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne v_back_porch \n"   // 2 if taken, 1 otherwise

   // Visible area: upper 240 lines
   ////////////////////////////////
   "ldi r25, 240 \n"        // 1
   "vis_upper: \n"
   "call draw_buffer \n"    // 4
   "call draw_wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne vis_upper \n"
    
   // Visible area: lower 240 lines
   ////////////////////////////////
   "ldi r25, 240 \n"        // 1
   "vis_lower: \n"
   "call draw_buffer \n"    // 4
   "call draw_wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne vis_lower \n"

   // Start over with new frame (v_sync)
   /////////////////////////////////////
   "add r18, r24 \n"        // 1, increment frame count
   "jmp loop_frame \n"      // 3
   
   
   // Subs
   "black_line: \n"
   // Wait 640+16 pixels (328 cycles), includes front porch
   "ldi r28, 109\n"         // 1 (327 to go)
   "wait_black: \n"
   "dec r28 \n"             // 1
   "brne wait_black \n"     // 2 taken, 1 else
   // At this point 1 + 109*3 - 1 cyles have passed (327)
   "delay2 \n"
   // Output sync in cycle 329, keep it for 32 cycles
   "out %[portb], r24 \n"   // 1
   "delay16 \n"
   "delay16 \n"
   "out %[portb], __zero_reg__ \n"   // 1
   // Back portch is 120 pixels (60 cycles)
   "ret \n"                 // 4 cycles, leaves 56 cycles of backporch to caller
   
   "black_line_clr_buf: \n"
   // 328 cycles till front porch, which we'll use to clear the linebuf array
   "ldi r26, lo8(%[linebuf]) \n" // 1  // load linebuffer ptr into X
   "ldi r27, hi8(%[linebuf]) \n" // 1
   "clrmem106 \n"                // 212
   "delay64 \n"
   "delay32 \n"
   "delay16 \n"
   "delay4 \n"
   "delay1 \n"
   // Output sync in cycle 329, keep it for 32 cycles
   "out %[portb], r24 \n"   // 1
   "delay16 \n"
   "delay16 \n"
   "out %[portb], __zero_reg__ \n"   // 1
   // Back portch is 120 pixels (60 cycles)
   "ret \n"                 // 4 cycles, leaves 56 cycles of backporch to caller
   
   "draw_buffer: \n"
   "out106 r21, %[portd] \n"       // 318 = 106 * 3
   "delay2 \n"                     // 2
   // Front porch, 16 pixels, 8 cycles
   "out %[portd], __zero_reg__ \n" // 1
   "delay8 \n"
   // Output sync in cycle 329, keep it for 32 cycles
   "out %[portb], r24 \n"          // 1
   "delay16 \n"
   "delay16 \n"
   "out %[portb], __zero_reg__ \n" // 1
   "ret \n"                        // 4 cycles, leaves 56 cycles of backporch to caller
   
   "wait52: \n"          // 4 for the call
   "ldi r28, 14\n"       // 1
   "loop_wait52: \n"
   "dec r28 \n"          // 1
   "brne loop_wait52 \n" // 2 taken, 1 else
   "delay2 \n"
   "ret \n"              // 4
   
   "wait51: \n"          // 4 for the call
   "ldi r28, 14\n"       // 1
   "loop_wait51: \n"
   "dec r28 \n"          // 1
   "brne loop_wait51 \n" // 2 taken, 1 else
   "delay1 \n"
   "ret \n"              // 4
   
   "prep_wait52: \n"             // 4 for the call
   "ldi r26, lo8(%[linebuf]) \n" // 1  // load linebuffer ptr into X
   "ldi r27, hi8(%[linebuf]) \n" // 1
   "ldi r28, 13\n"               // 1
   "loop_prep_wait52: \n"
   "dec r28 \n"                  // 1
   "brne loop_prep_wait52 \n"    // 2 taken, 1 else
   "delay2 \n"
   "delay1 \n"
   "ret \n"                      // 4

   // r25 has linenum
   "draw_wait52: \n"             // 4 for the call
   
   // calculate the offset for this line
   "mov r17, r20 \n"             // 1, copying line number
   "ldi r19, 32 \n"              // 1, adding constant
   "add r17, r19 \n"             // 1
   "lsr r17 \n"                  // 1, div 2

   "mov r26, r20 \n"             // 1, move line number into offset
   "add r26, r18 \n"             // 1, add frame count
   "cbr r26, 0x80 \n"            // 1, mod 128 (this is X low)
   "ldi r27, hi8(%[sinetab]) \n" // 1, hi byte of sinetab
   "ld r19, X \n"                // 2, load value from sinetab using offet in r26
   
   "muls r17, r19 \n"            // 2
   "mov r28, r1 \n"              // 1, the high byte is what we need
   "ldi r17, 0 \n"               // 1
   "mov __zero_reg__, r17 \n"    // 1, clean up __zero_reg__ again, which has been used by muls
   
   "ldi r17, 51 \n"              // 1, add 51 to center the bars
   "add r28, r17 \n"             // 1
   // 17 cycles
   
   // calculate color based on lumtab with offset in r26
   "ldi r27, hi8(%[lumtab_grn]) \n"  // 1, hi byte of lumtab
   "ld r19, X \n"                    // 2, load value from lumtab using offet in r26
   "ldi r27, hi8(%[lumtab_blu]) \n"  // 1, hi byte of lumtab
   "ld r17, X \n"                    // 2, load value from lumtab using offet in r26
    
   // linebuffer is aligned on 0x100 boundaries, so we can just use the low byte as offset
   "mov r26, r28 \n"             // 1
   "ldi r27, hi8(%[linebuf]) \n" // 1
   
   // Drawing
   "st X+, r17 \n"               // 2, blue
   "st X+, r17 \n"               // 2, blue
   "st X+, r5 \n"                // 2, white
   "st X+, r19 \n"               // 2, green
   "st X+, r19 \n"               // 2, green

   // Increment logical line every 4 scanlines 
   "ldi r19, 3 \n"               // 1, load bitmask (optimize later)
   "and r19, r25 \n"             // 1, mask two LSBs
   "cpi r19, 1 \n"               // 1, last two bits '01'? Raster line counts down, every 4th line matches
   "brne no_incr \n"             // don't increment otherwise
   "add r20, r24 \n"             // increment logical line
   "no_incr: \n"                 // 2 total
   
   "ldi r26, lo8(%[linebuf]) \n" // 1  // load linebuffer ptr into X
   "ldi r27, hi8(%[linebuf]) \n" // 1
   
   "delay2 \n"
   "delay1 \n"
   "ret \n"                      // 4

   : // output operands
   : // input operands
   [linebuf] "i" (linebuffer),
   [sinetab] "i" (sinetab),
   [lumtab_grn] "i" (lumtab_grn),
   [lumtab_blu] "i" (lumtab_blu),
   [portb] "I" (_SFR_IO_ADDR(PORTB)),
   [portc] "I" (_SFR_IO_ADDR(PORTC)),
   [portd] "I" (_SFR_IO_ADDR(PORTD))
   : // clobbered registers
   "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27"
   );
}
