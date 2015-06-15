#include "macros.i"
#include "pins.i"

	.global start

start:
	ldi r26, lo8(linebuf) 	; load linebuffer ptr into X
	ldi r27, hi8(linebuf)

	.macro lldi rd, imm
	ldi r19, \imm
	mov \rd, r19
	.endm
	
	;; Keeping some numbers handy
	ldi r19, 2		; ldi needs registers > r15
	mov r2, r19		; r2 <= 2
	ldi r19, 4
	mov r3, r19		; r3 <= 4
	ldi r19, 6
	mov r4, r19		; r4 <= 6
	ldi r19, 127
	mov r5, r19		; r5 <= 127
	lldi r9, 128
	lldi r10, 32
	
	;; Used as colors for bottom text (fixme: combine with above)
	lldi r6, 2
	lldi r7, 4
	lldi r8, 6
	
	ldi r24, 1
	ldi r23, 3
	ldi r22, 7
   
	// Other registers used:
	// r25 -- line counter
	// r28 -- delay/column counter or offset
	// r21 -- output color
	// r20 -- logical line, increment every 4 raster lines
	// r19 -- misc
	// r18 -- frame counter/time
	// r17 -- misc
	// r15 -- front delay for text
	// r14 -- back delay for text
   
	ldi r18, 0
      
loop_frame:
	;; Cycle budget for each scanline is 420; we are 4 cycles over budget

	;;  Starting with vsync output

	;; 
	;; Vsync front porch: 1 line
	;; 
	call black_line		; 369
	call wait_bp		; 52
	delay1			; 1
        			; 1 + 1 (for next two instructions)
	;;  ==> 424 (+4)
   
	;;
	;; Vsync pulse: 3 lines
	;; 
	out PORTC, r10		; (1), in last line's BP
	ldi r25, 3		; (1), ditto
v_sync_pulse:
	call black_line		; 369
	call wait_bp		; 52
	dec r25			; 1
	brne v_sync_pulse	; 2 if taken, 1 otherwise
	out PORTC, __zero_reg__	; 1 cycle (equals out with untaken branch)
	;;  ==> 424 (+4)

	;; 
	;; Vsync back porch: 16 lines
	;; 
	;; First line: clear buffer
	call black_line_clr_buf ; 369
	call prep_wait_bp      	; 52
	ldi r20, 0             	; 1, set logical line back to 0
	delay1                 	; 1
				; 1 (for next instruction)
	;; ==> 424 (+4)
   
	;; 15 more lines
	ldi r25, 15         	; (1), accounted for above
v_back_porch:
	call black_line		; 369
	call prep_wait_bp	; 52
	dec r25			; 1
	brne v_back_porch	; 2 if taken, 1 otherwise
				; 1, next instruction on untaken branch
	;; ==> 424 (+4)

	;; 
	;; Visible area: upper 240 lines
	;; 
	ldi r25, 252		; (1), accounted for above
vis_upper:
	call draw_buffer_rbars	; 369
	call draw_wait_bp	; 52
	dec r25		        ; 1
	brne vis_upper	  	; 2 if taken, 1 otherwise
				; 1, next instruction on untaken branch
	;; ==> 424 (+4)
   
	;;
	;; Visible area: lower 240-5 lines
	;; 
	ldi r25, 223		; (1), accounted for above
vis_lower:
	call draw_buffer	; 369
	call draw_wait_bp	; 52
	dec r25			; 1
	brne vis_lower		; 2 if taken, 1 otherwise
	delay1			; 1, 2 total for brne
	;; ==> 424 (+4)

	delay1			; 1, timing fix, xxx review this
	
	;; Macro to call a hardcoded line of output
	.macro hardline sub
	delay2			; 2, account for memory access in previous scanlines

	delay32
	delay8
	mov r19, r15		; 1
	vardelay r19, l1_\sub	; 7 + 0..63
	call \sub		; 111 (107 + 4)
	mov r19, r14		; 1
	vardelay r19, l2_\sub	; 7 + 63..0
	;; vardelay total: 14 + 127 = 141
	;; 2 add'l mov cycles

	delay16
	delay8
	delay2
	delay1
	front_porch_and_sync	; 41
	delay4			; 4
	call prep_wait_bp	; 52
	.endm

	hardline txt_line0
	delay4
	hardline txt_line1
	delay4
	hardline txt_line2
	delay4
	hardline txt_line3
	delay4
	hardline txt_line4
	;; No delay for last line to account for 4 cycles below

	;; 
	;; Start over with new frame (v_sync)
	;; 
	add r18, r24		; 1, increment frame count
	jmp loop_frame		; 3
