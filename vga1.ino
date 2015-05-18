// VGA output on a 16MHz Arduino Uno for the VESA 640x480@75Hz video mode

// Pin mappings for VGA; colors are on portd, horizontal sync is on portb,
// vertical sync is on portc
#define H_SYNC 8  // PB0
#define V_SYNC A0 // PC0
#define RED 0     // PD0
#define GRN 1     // PD1
#define BLU 2     // PD2

// 106 pixels width for the line buffer, each pixel in the buffer corresponds
// to 6 pixels in the VGA output.
#define LINEWIDTH 106
volatile uint8_t linebuffer[LINEWIDTH]; // declare as volatile since asm code will change this

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

void init_gfx() {
  size_t i;
  for(i = 0; i < LINEWIDTH; ++i) {
    if(i < 5) {
      linebuffer[i] = 2;  // green
    } else if(i < 10) {
      linebuffer[i] = 3;  // yellow
    } else if(i < 15) {
      linebuffer[i] = 6;  // cyan
    } else if(i < 20) {
      linebuffer[i] = 0;  // black
    } else if(i < 25) {
      linebuffer[i] = 5;  // purple
    } else if(i < 60) {   // alternating blue/green
      if(i % 2) {
        linebuffer[i] = 4;
      } else {
        linebuffer[i] = 2;
      }
    } else {              // alternating red/white
      if(i % 2) {
        linebuffer[i] = 1;
      } else {
        linebuffer[i] = 7;
      }    }
  }
}

void setup() {
  pinMode(H_SYNC, OUTPUT);
  pinMode(V_SYNC, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GRN, OUTPUT);
  pinMode(BLU, OUTPUT);
  
  init_gfx();
  cli();
}

void loop() {
   __asm__ __volatile__ (
   "ldi r26, lo8(%[linebuf]) \n"    // load linebuffer ptr into X
   "ldi r27, hi8(%[linebuf]) \n"
   
   // Keeping some numbers handy
   "ldi r24, 1 \n"
   "ldi r23, 3 \n"
   "ldi r22, 7 \n"
   
   // Other registers used:
   // r25 -- line counter
   // r28 -- delay/column counter
   // r21 -- output color
      
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
   "ldi r25, 16 \n"         // 1
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
   "call prep_wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne vis_upper \n"
    
   // Visible area: lower 240 lines
   ////////////////////////////////
   "ldi r25, 240 \n"        // 1
   "vis_lower: \n"
   "call draw_buffer \n"    // 4
   "call prep_wait52 \n"
   "nop \n"                 // 1
   "dec r25 \n"             // 1
   "brne vis_lower \n"

   // Start over with new frame (v_sync)
   /////////////////////////////////////
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
   
   : // output operands
   : // input operands
   [linebuf] "i" (linebuffer),
   [portb] "I" (_SFR_IO_ADDR(PORTB)),
   [portc] "I" (_SFR_IO_ADDR(PORTC)),
   [portd] "I" (_SFR_IO_ADDR(PORTD))
   : // clobbered registers
   "r21", "r22", "r23", "r24", "r25", "r26", "r27"
   );
}
