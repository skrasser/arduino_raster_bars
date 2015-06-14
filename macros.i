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

	;; Fast memory clear, i.e. w/o loop (memory pointed to by X)
	.macro clrmem1
	st X+, r1		; 2
	.endm
  
	.macro clrmem2
	clrmem1
	clrmem1
	.endm

	.macro clrmem4
	clrmem2
	clrmem2
	.endm

	.macro clrmem8
	clrmem4
	clrmem4
	.endm

	.macro clrmem16
	clrmem8
	clrmem8
	.endm

	.macro clrmem32
	clrmem16
	clrmem16
	.endm

	.macro clrmem64
	clrmem32
	clrmem32
	.endm

	.macro clrmem106
	clrmem64
	clrmem32
	clrmem8
	clrmem2
	.endm

	
	.macro front_porch_and_sync colport, syncport
	;;  Front porch, 16 pixels, 8 cycles
	out \colport, r1
	delay4
	delay2
	delay1
	;; Output sync in cycle 329, keep it for 32 cycles
	out \syncport, r24
	delay16
	delay8
	delay4
	delay2
	delay1
	out \syncport, r1
	;; Total for macro: 41 cycles
	.endm

