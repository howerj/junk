;; hexdump.asm: Copyright (C) 2001 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o hexdump hexdump.asm && chmod +x hexdump
;;
;; Usage: hexdump [FILE]
;; If FILE is omitted, hexdump reads from standard input.
;; hexdump returns zero on success, or an errno code on error.

BITS 32

;; Number of bytes displayed per line.

%define linesize 16

;; The executable's ELF header and program header table, which overlap
;; each other slightly. The program header table defines a single
;; segment of memory, with both write and execute permissions set,
;; which is loaded with contents of the entire file plus enough space
;; after to hold the data section, as defined above. The entry point
;; for the program appears within the program header table, at an
;; unused field.

		org	0x4B5B0000

		db	0x7F, "ELF"		; e_ident
		db	1, 1, 1, 0
		dd	0
		dd	0
		dw	2			; e_type
		dw	3			; e_machine
		dd	1			; e_version
		dd	_start			; e_entry
		dd	phdr - $$		; e_phoff
		dd	0			; e_shoff
		dd	0			; e_flags
		dw	0x34			; e_ehsize
		dw	0x20			; e_phentsize
phdr:		dw	1			; e_phnum	; p_type
		dw	0			; e_shentsize
		dw	0			; e_shnum	; p_offset
		dw	0			; e_shstrndx
		dw	0					; p_vaddr
_start:		pop	ebx
		dec	ebx
		jle	short usestdin				; p_paddr
		pop	ebx
		cmp	eax, filesize				; p_filesz
		jmp	short proceed				; p_memsz
		dw	0
		dd	7					; p_flags
		dd	0x1000					; p_align

;; The code within the program header table, above, removes argv[0]
;; from the stack and examines argc. If argc is one (i.e. no argument
;; on the command line), the program skips ahead, leaving eax set to
;; zero, indicating standard input. Otherwise, argv[1] is retrieved
;; and the open system call is invoked to get a file descriptor, which
;; is stored on the stack.

proceed:	pop	ebx
		mov	al, 5
		int	0x80
		or	eax, eax
		js	short done
usestdin:	push	eax

;; Loop once for each line of output.

lineloop:

;; edi is initialized to the location of the output buffer, and the
;; left-hand side is filled with spaces. Afterwards edi will be
;; pointing to the rightmost column of the output buffer, which is
;; also the location of the input buffer.

		mov	edi, lineout
		push	edi
		push	byte leftsize
		pop	ecx
		mov	al, ' '
		rep stosb

;; edi is reset to point to the start of the output buffer. The read
;; system call is invoked, and up to linesize bytes are pulled from
;; the input file. If an error occurs or no bytes are read, the
;; program exits the loop.

		pop	eax
		xchg	eax, edi
		xchg	eax, ecx
		mov	dl, linesize
		pop	ebx
		push	ebx
		mov	al, 3
		int	0x80
		or	eax, eax
		jle	short done

;; eax contains the number of bytes actually read from the input file.
;; This value is saved in ecx, and esi is pointed to the input buffer.
;; ebx is reset to one.

		mov	bl, 1
		pusha
		xchg	eax, ecx
		xchg	eax, esi

;; The current file offset is rendered in ASCII hexdecimal in the
;; leftmost column of the output buffer. (The call to rhexout calls
;; hexout three times in a row, rendering the highest three bytes and
;; destroying eax in the process. eax is then restored and the lowest
;; byte of the offset is rendered by calling hexout directly.)
;; Finally, a colon is appended.

		mov	eax, ebp
		lea	edx, [byte esi - linein + rhexout]
		call	edx
		xchg	eax, ebp
		mov	dl, hexout - $$
		call	edx
		mov	al, ':'
		stosb

;; At every even byte, edi is incremented, leaving a space in the
;; output buffer. Each byte value is passed to hexout, to add the
;; hexadecimal ASCII representation to the output buffer. Bytes with
;; values outside the range 0x20-0x7E are replaced with a dot in the
;; input buffer. The file offset is incremented, and the loop repeats
;; for each byte in the input buffer.

byteloop:
		add	edi, ebx
		xor	ebx, byte 1
		lodsb
		inc	eax
		cmp	al, ' '
		jg	short graphic
		mov	[byte esi - 1], byte '.'
graphic:	dec	eax
		call	edx
		loop	byteloop

;; A newline is appended, and the entire buffer is passed to the write
;; system call. If no errors occur, the program then loops back to
;; process the next line of input.

		mov	byte [esi], 10
		popa
		mov	cl, 0
		lea	edx, [byte eax + leftsize + 1]
		add	ebp, eax
		mov	al, 4
		int	0x80
		or	eax, eax
		jge	short lineloop

;; The program comes here when it's time to leave. eax will either be
;; zero, indicating a successful run, or contain a negative error
;; code. This value is negated and the exit system call is invoked.

done:
		neg	eax
		xchg	eax, ebx
		xor	eax, eax
		inc	eax
		int	0x80

;; rhexout is called via edx, so by pushing edx before falling through
;; to the actual subroutine, hexout, it will return to this spot. edx
;; will then be pushed again; however, this time edx will have been
;; incremented, so when it returns the second time, the stack will
;; remain unmodified, and the next return will actually return to the
;; caller. As a result, hexout is executed three times in a row, and
;; edx will have been advanced a total of three bytes. eax is rotated
;; left one byte before each execution, so hexout operates on
;; successively less significant bytes of eax. (rhexout only runs
;; through three of the bytes since the first execution destroys the
;; value of the fourth byte of eax.)

rhexout:
		push	edx
		inc	edx
		rol	eax, 8

;; The hexout subroutine stores at edi an ASCII representation of the
;; hexadecimal value in al. Both al and ah are destroyed in the
;; process. edi is advanced two bytes.

hexout:

;; The first instruction breaks apart al into two separate nybbles,
;; one each in ah and al. The high nybble is handled first, then when
;; the digitout "subroutine" returns, the low nybble is handled, and
;; hexout returns to the real caller.

		aam	16
		call	digitout
digitout:	xchg	al, ah

;; The compare and sbb instructions will change values of 0 through 9
;; to 96 through 9F (150-159 in decimal), and values of 10 through 15
;; to A1 through A6 (161-166 in decimal). The das instruction will
;; subtract 6 from each nybble (except in the last case, where there
;; was no internal carry), leaving values in the range 30-39,41-46.
;; This value is stored in the output buffer.

		cmp	al, 10
		sbb	al, 0x69
		das
		stosb
		ret

filesize equ $ - $$

;; The size and shape of the program's data buffer.

ABSOLUTE $$ + 0x0100

;; Each line of the program's output has the following format:
;;
;; FILEOFFS: HEXL HEXL HEXL HEXL HEXL HEXL HEXL HEXL  ASCII.CHARACTERS
;;
;; The ASCII region of the output, at far right, also doubles as the
;; input buffer.

lineout:
		resb	8			; 8 characters for the offset
		resb	1			; a colon
		resb	5 * linesize / 2	; the hex byte display
		resb	2			; two spaces
linein:		resb	linesize		; the ASCII characters
		resb	1			; a newline character

leftsize equ linein - lineout
