#include "pins.i"
#define __zero_reg__ r1

	;; Delay macros
	.macro	delay1
	nop
	.endm

	.macro	delay2
	delay1
	delay1
	.endm

	.macro	delay4
	delay2
	delay2
	.endm

	.macro	delay8
	delay4
	delay4
	.endm
	
	.macro	delay16
	delay8
	delay8
	.endm

	.macro	delay32
	delay16
	delay16
	.endm

	.macro	delay64
	delay32
	delay32
	.endm

	;; Variable delay based on reg
	.macro vardelay reg, lab
	lsr \reg		; 1, puts LSB into C
	brcs first_\lab		; 2 taken, 1 not taken
first_\lab:
	;; 2 cycles
	;; +1 if bit0 = 1

	lsr \reg		; 1
	brcc second_\lab	; 2/1
	delay1			; 1, match branch taken cycle count
	delay2			; 2 more cycles
second_\lab:
	;; 3 cycles
	;; +2 if bit1 = 1

	breq done_\lab		; exit if zero
	delay1			; total: 2
	;; totals:
	;; 7 cycles
	;; +1 if bit0 set, +2 if bit1 set
	
loop_\lab:			; 4 cycles for each iteration
	delay1
	dec \reg
	brne loop_\lab
	delay1			; 1, compensate for brne not taken
	
done_\lab:
	;; grand total:
	;; 7 + reg cycles
	.endm
	
	;; Output macros
	.macro out1 reg,port
	ld \reg, X+		; 2
	out \port, \reg		; 1
	.endm
  
	.macro out2 reg,port
	out1 \reg, \port
	out1 \reg, \port
	.endm
  
	.macro out4 reg,port
	out2 \reg, \port
	out2 \reg, \port
	.endm

	.macro out8 reg,port
	out4 \reg, \port
	out4 \reg, \port
	.endm

	.macro out16 reg,port
	out8 \reg, \port
	out8 \reg, \port
	.endm

	.macro out32 reg,port
	out16 \reg, \port
	out16 \reg, \port
	.endm
  
	.macro out64 reg,port
	out32 \reg, \port
	out32 \reg, \port
	.endm

	.macro out106 reg,port
	out64 \reg, \port
	out32 \reg, \port
	out8 \reg, \port
	out2 \reg, \port
	.endm

	;; Fast memory fill, i.e. w/o loop (memory pointed to by X)
	.macro fillmem1 reg
	st X+, \reg		; 2
	.endm
  
	.macro fillmem2 reg
	fillmem1 \reg
	fillmem1 \reg
	.endm

	.macro fillmem4 reg
	fillmem2 \reg
	fillmem2 \reg
	.endm

	.macro fillmem8 reg
	fillmem4 \reg
	fillmem4 \reg
	.endm

	.macro fillmem16 reg
	fillmem8 \reg
	fillmem8 \reg
	.endm

	.macro fillmem32 reg
	fillmem16 \reg
	fillmem16 \reg
	.endm

	.macro fillmem64 reg
	fillmem32 \reg
	fillmem32 \reg
	.endm

	.macro fillmem106 reg
	fillmem64 \reg
	fillmem32 \reg
	fillmem8 \reg
	fillmem2 \reg
	.endm

	.macro delay7
	delay4
	delay2
	delay1
	.endm

	.macro delay31
	delay16
	delay8
	delay4
	delay2
	delay1
	.endm
	
	.macro front_porch_and_sync_work busy7, busy31, colreg
	;; Front porch, 16 pixels, 8 cycles
	;; Pin 7 to high (disables background color), primary colors to zero
	out PORTD, r9
	\busy7
	;; Output sync in cycle 329, keep it for 32 cycles
	out PORTB, r10
	\busy31
	;; For the horizontal raster bars, we need to set the color once we've
	;; removed the sync signal
	out PORTB, \colreg
	;; Total for macro: 41 cycles
	.endm

	.macro front_porch_and_sync
	front_porch_and_sync_work delay7, delay31, r1
	.endm

