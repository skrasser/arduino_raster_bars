#include "pins.i"
#include "macros.i"
	
	.global black_line
	.global black_line_clr_buf
	.global draw_buffer

	.global wait_bp
	.global prep_wait_bp
	.global draw_wait_bp

	;;
	;; Subs to draw the visible area, front porch, and sync of a scanline
	;; Expect r1=0, r24=1
	;;

	;; Draws a black line
black_line: 			; 369 incl. call	
	delay64
	delay64
	delay64
	delay64
	delay64			; this takes us to cycle 320   
	front_porch_and_sync PORTD, PORTB	; 41
	;;  Back porch is 120 pixels (60 cycles), 59 more
	ret			; 4 cycles, leaves 55 cycles of backporch to caller

	;; Draws a black line while zeroing out linebuf, calculates
	;; offsets for text at bottom of screen based on sinetab
	;; Sets: r14, r15 (offsets)
	;; Clobbers: r26, r27 (X), r19
black_line_clr_buf:		; 369 incl. call
	;;  328 cycles till front porch, which we'll use to clear the linebuf array
	ldi r26, lo8(linebuf)	; 1, load linebuffer ptr into X
	ldi r27, hi8(linebuf)	; 1
	clrmem106		 	; 212

	mov r19, r18		; 1, copy frame count
	lsr r19			; 1, 0..127

	ldi r27, hi8(sinetab)	; 1
	mov r26, r19		; 1, use as offset to sinetab
	ld r19, X		; 2, load value from sinetab
	asr r19			; 1, divide by 2
	subi r19, -63		; 1, add 63 to get all positive sine
	
	mov r15, r19		; 1, keep for later
	ldi r17, 127		; 1
	sub r17, r19		; 1
	mov r14, r17		; 1, keep for later
	
	delay64
	delay16
	delay8
	delay4
	delay1			; this takes us to cycle 320
	front_porch_and_sync PORTD, PORTB 	; 41
	;; Back portch is 120 pixels (60 cycles), 59 more
	ret			; 4 cycles, leaves 55 cycles of backporch to caller

	;; Output 106 bytes pointed to by X
	;; Clobbers: r21
draw_buffer:			; 369 including call
	out106 r21, PORTD	; 318 = 106 * 3
	delay2
	front_porch_and_sync PORTD, PORTB	; 41
	ret			; 4 cycles, leaves 55 cycles of backporch to caller

	;;
	;; Subs to wait and do work while outputting the back porch of a
	;; scanline. Calling these functions takes 52 (including 4 cycles
	;; taken for the call instruction).
	;;

	;; Wait 52 cycles
	;; Clobbers r28
wait_bp:			; 4 for the call
	ldi r28, 14		; 1
loop_wait52:
	dec r28			; 1
	brne loop_wait52	; 2 taken, 1 else
	delay2			; 2
	ret			; 4

   
prep_wait_bp:             	; 4 for the call
	ldi r26, lo8(linebuf) 	; 1, load linebuffer ptr into X
	ldi r27, hi8(linebuf) 	; 1
	ldi r28, 13		; 1
loop_prep_wait52:
	dec r28			; 1
	brne loop_prep_wait52	; 2 taken, 1 else
	delay2			; 2
	delay1			; 1
	ret			; 4

	;; Update the scanline buffer during the back porch
	;; r25 has linenum
draw_wait_bp:			; 4 for the call
	;;  calculate the offset for this line
	mov r17, r20		; 1, copying line number
	ldi r19, 32		; 1, adding constant
	add r17, r19		; 1
	lsr r17	                ; 1, divide by 2

	mov r26, r20		; 1, move line number into offset
	add r26, r18		; 1, add frame count
	cbr r26, 0x80		; 1, mod 128 (this is X low)
	ldi r27, hi8(sinetab)	; 1, hi byte of sinetab
	ld r19, X		; 2, load value from sinetab using offet in r26
   
	muls r17, r19		; 2
	mov r28, r1		; 1, the high byte is what we need
	ldi r17, 0		; 1
	mov __zero_reg__, r17	; 1, clean up __zero_reg__ again, which has been used by muls
   
	ldi r17, 51		; 1, add 51 to center the bars
	add r28, r17		; 1
	;;  17 cycles
   
	;; calculate color based on lumtab with offset in r26
	ldi r27, hi8(lumtab_grn) ; 1, hi byte of lumtab
	ld r19, X		 ; 2, load value from lumtab using offet in r26
	ldi r27, hi8(lumtab_blu) ; 1, hi byte of lumtab
	ld r17, X		 ; 2, load value from lumtab using offet in r26
    
	;; linebuffer is aligned on 0x100 boundaries, so we can just use the low byte as offset
	mov r26, r28		; 1
	ldi r27, hi8(linebuf)	; 1
   
	;; drawing
	st X+, r17		; 2, blue
	st X+, r17		; 2, blue
	st X+, r5               ; 2, white
	st X+, r19		; 2, green
	st X+, r19		; 2, green

	;; increment logical line every 4 scanlines 
	ldi r19, 3		; 1, load bitmask (optimize later)
	and r19, r25 		; 1, mask two LSBs
	cpi r19, 1 		; 1, last two bits '01'? Raster line counts down, every 4th line matches
	brne no_incr		; don't increment otherwise
	add r20, r24		; increment logical line
	no_incr:		; 2 total
   
	ldi r26, lo8(linebuf)	; 1, load linebuffer ptr into X
	ldi r27, hi8(linebuf)	; 1
   
	delay2			; 2
	ret			; 4
