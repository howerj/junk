	;; Acid - 256 byte i386 Linux demo by Johan B/ NoCrew.
	;;         ELF headers by Brian Raiter.
	;;        $ nasm -f bin -o acid acid.asm
	
		BITS 32

lines		equ	13
max_shift	equ	6	
columns		equ	(13 + max_shift)
term_clr_size	equ	10
data_size	equ	(lines * (columns + 1))

		org	0x08048000

		; Elf32_Ehdr and Elf32_Phdr overlaps with 8 bytes.

ehdr:		db      0x7F, "ELF", 1, 1, 1, 0	; e_ident
		dd      0
		dd      0
		dw      2			; e_type
		dw      3			; e_machine
		dd      1			; e_version
		dd      _start			; e_entry
		dd      phdr - $$		; e_phoff
		dd      0			; e_shoff
		dd      0			; e_flags
		dw      ehdrsize		; e_ehsize
		dw      phdrsize		; e_phentsize
phdr:		dw      1			; e_phnum	; p_type
		dw      0			; e_shentsize
		dw      0			; e_shnum	; p_offset
		dw      0			; e_shstrndx
ehdrsize	equ	$ - ehdr
		dd	$$			; p_vaddr
		dd      $$			; p_paddr
		dd      filesize		; p_filesz
		dd      filesize + data_size	; p_memsz
		dd      7			; p_flags ; RW (5 = R)
		dd      0x1000			; p_align
phdrsize	equ     $ - phdr
		
_start					; Program start.

		xor	esi,esi		; Base shift index

f_loop		mov	edi,datasection	; Data address
		mov	ebp,lines-1	; Line counter

y_loop		mov	dl,columns-1	; Column counter

		lea	ecx,[ebp+esi]	; Look up shift value
		and	ecx,0xF
		mov	cl,[shift_t+ecx]

		xor	ebx,ebx		; Prepare acid 1 bpp line
		mov	bx,[acid+ebp*2]
		shl	ebx,cl		; Must use cl register in shift:(

x_loop		mov	al," "		; Space character
		shr	ebx,1
		jnc	space
		mov	al,"*"		; Star character
space		mov	[edi],al
		inc	edi
		sub	dl,1
		jnc	x_loop

		mov	[edi],byte 10	; Line feed
		inc	edi
		sub	ebp,1
		jnc	y_loop

		xor	eax,eax		; Write acid:)
		mov	al,4
		xor	ebx,ebx
		inc	ebx
		mov	ecx,term_clr
		mov	edx,data_size + term_clr_size
		int	0x80

		xor	eax,eax		; nanosleep()
		mov	al,162
		mov	ebx,nano_t
		xor	ecx,ecx
		int	0x80
		
		inc	esi
		jmp	short f_loop	; Loop forever

		;xor	eax,eax		; Exit program.
		;inc	eax
		;int	0x80


shift_t		db	3,4,5,6,6,6,5,4	; Shift table 
		db	3,2,1,0,0,0,1,2

acid		dw	0000111110000b
		dw	0011000001100b
		dw	0100000000010b
		dw	0100111110010b
		dw	1001100011001b
		dw	1001000001001b
		dw	1000000000001b
		dw	1000000000001b
		dw	1000110110001b
		dw	0100110110010b
		dw	0100000000010b
		dw	0011000001100b
		dw	0000111110000b

nano_t		dd	0,50000000	; Input to nanosleep()

	;; Clear screen VT100 codes
term_clr	db	27,91,50,74,27,91,49,59,49,72

datasection

filesize	equ     $ - $$

