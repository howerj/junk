		/* ARM Linux version of the ACID demo.
		 * Not quite 256 bytes for various reasons,
		 * but it's still small.
		 */

lines		=	13
max_shift	=	6	
columns		=	(13 + max_shift)
term_clr_size	=	10
output_size	=	(lines * (columns + 1))


/* We use a data segment because we need to write to it.
 */
.data
	.global	_start
_start:	

/*
r0	y counter
r1	x counter
r2	tmp
r3	shift table
r4	acid
r5	output
r6	shift counter
r7	tmp
*/

	
f_loop:	adr	r5,output
	adr	r3,shift_t
	adr	r4,acid
	mov	r0,#lines-1		@ Y counter
	
y_loop:	mov	r1,#columns-1		@ X counter

	add	r2,r0,r6		@ Look up shift value
	and	r2,r2,#0xF
	ldrb	r2,[r3,r2]

	add	r7,r4,r0,LSL #1		@ No shift with half word loads.
	ldrh	r7,[r7]			@ Look up 1 bpp acid line
	mov	r7,r7,LSL r2
	
x_loop:	movs	r7,r7,LSR #1
	movcc	r2,#' '			@ ARM power:)
	movcs	r2,#'*'
	strb	r2,[r5],#1
	subs	r1,r1,#1
	bge	x_loop

	mov	r2,#10
	strb	r2,[r5],#1
	subs	r0,r0,#1
	bge	y_loop
	
	mov	r0,#1			@ Write ACID
	adr	r1,term_clr
	ldr	r2,write_size		@ To big for immediate.
	swi	0x900004

	adr	r0,nano_t		@ Sleep
	mov	r1,r0
	swi	0x9000a2

	add	r6,r6,#1
	b	f_loop			@ Loop forever

	/*	
	mov	r0,#22			@ Exit program.
	swi	0x900001
	*/

	
nano_t:	.word	0,50000000		@ Input to nanosleep()

write_size:
	.word	output_size + term_clr_size
	
acid:	.short	0b0000111110000
	.short	0b0011000001100
	.short	0b0100000000010
	.short	0b0100111110010
	.short	0b1001100011001
	.short	0b1001000001001
	.short	0b1000000000001
	.short	0b1000000000001
	.short	0b1000110110001
	.short	0b0100110110010
	.short	0b0100000000010
	.short	0b0011000001100
	.short	0b0000111110000

shift_t:
	.byte	3,4,5,6,6,6,5,4		@ Shift table 
	.byte	3,2,1,0,0,0,1,2

term_clr:
	.byte	27,91,50,74,27,91,49,59,49,72	@ Clear VT100 screen

output:	.space	output_size
