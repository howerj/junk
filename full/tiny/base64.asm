;; base64.asm: Copyright (C) 2009 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o base64 base64.asm && chmod +x base64
;;
;; Usage: base64 [FILE]
;; base64 reads base64-encoded text from FILE, or from standard input
;; if FILE is omitted, and writes the decoded bytes to stdout.
;; The exit status is zero on success, or an errno code on error.

BITS 32

;; Maximum size of the input buffer.

%define BUFSIZ 8192

;; The executable's ELF header and program header table, which overlap
;; each other slightly. The program header table defines a single
;; segment of memory, with both write and execute permissions set,
;; which is loaded with contents of the entire file plus enough space
;; after to hold the program's I/O buffers. The entry point for the
;; program appears within the program header table. The initial code
;; sets eax to 5, edi to 256, removes argc, argv[0], and argv[1] from
;; the stack, and sets the zero flag if argc is one.

		org	0x5B05B000

		db	0x7F, "ELF", 1, 1, 1, 0	; e_ident
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
		db	0					; p_vaddr
_start:		mov	al, 5
		pop	ebx
		dec	ebx					; p_paddr
		pop	ebx
		pop	ebx
		mov	edi, 256				; p_filesz
		jmp	short setup				; p_memsz
		dw	0
		dd	7					; p_flags
		dd	0x1000					; p_align

;; The program jumps here to reenter the inner loop after the input
;; buffer has been refilled. The buffer size count is moved to ebx,
;; while eax has its high bytes cleared.

resume:
		xchg	eax, ebx

;; The program's inner loop. Successive bytes are retrieved from the
;; input buffer and examined. Byte values less than '+' (e.g.
;; whitespace characters) are ignored. The remaining values are looked
;; up in hextable and the bit pattern is appended to edi. When four
;; such values have been accumulated, the three bytes so formed are
;; added to the output buffer, and edi is cleared. (Note that the
;; hextable entry for '=' has the value 0xFF, which causes the overall
;; size of the output buffer to be decremented.) The loop continues
;; until the input buffer's content is exhausted.

byteloop:
		mov	al, [ecx]
		inc	ecx
		cmp	al, '+'
		jl	short nextchar
		mov	al, [hextable + eax]
		cmp	al, 0xFE
		adc	ebp, byte -1
		or	edi, eax
		shl	edi, 6
		jnc	short nextchar
		bswap	edi
		mov	[edx], edi
		lea	edx, [byte edx + ebp + 3]
		pop	edi
		push	edi
nextchar:	inc	ebx
		jnz	short byteloop

;; ebx is incremented to one, indicating stdout, and the contents of
;; the output buffer are passed to the write system call. write is
;; called repeatedly until the entire buffer has been output (or until
;; an error is returned, in which case the program exits).

		inc	ebx
		sub	edx, ecx
writeloop:	lea	eax, [byte ebx + 3]
		int	0x80
		neg	eax
		jns	short errorout
		sub	ecx, eax
		add	edx, eax
		jnz	short writeloop

;; ecx is pointed to the start of the input buffer and the read system
;; call is invoked. If a positive value is returned, then edx is
;; pointed to output buffer, located just past the input, and the
;; program jumps into the inner loop.

entry:		lea	eax, [byte edx + 3]
		mov	ebx, esi
		mov	ecx, buffer
		mov	dh, BUFSIZ / 256
		int	0x80
		lea	edx, [ecx + eax]
		neg	eax
		jl	short resume

;; The program comes here on error (or when stdin returns EOF, in
;; which case eax will be zero). The value in eax is passed to the
;; exit system call.

errorout:	xchg	eax, ebx
		xor	eax, eax
		inc	eax
		int	0x80

;; The initialization code continues here. edi's starting value is
;; stored on the stack so it can be retrieved when needed. If an
;; argv[1] is present, the program passes it to the open system call
;; to get a file descriptor, which is stored in esi. (If no arguments
;; are present, then esi will just be zero, the file descriptor for
;; stdin.) The program then enters the main loop.

setup:
		push	edi
		jle	short entry
		int	0x80
		xchg	eax, esi
		sub	eax, esi
		jg	short errorout
		jmp	short entry


;; The base64 translation table. Only the ASCII values between 43 and
;; 127 inclusive are given in this table -- other values are filtered
;; out before lookup occurs. Each entry provides a bit pattern which
;; is stored in the upper six bits. The bottom two bits of each entry
;; must be cleared. The exception to this rule is the entry for the
;; equal sign -- this is set to 0xFF to indicate that this value does
;; not contribute to the program's final output. Note that the entries
;; for plus and slash are duplicated by the dash and underscore
;; entries; this supports a variant of the standard base64 encoding
;; created for use with filenames.

hextable equ $ - '+'

		db	0xF8, 0, 0xF8, 0, 0xFC
		db	0xD0, 0xD4, 0xD8, 0xDC, 0xE0
		db	0xE4, 0xE8, 0xEC, 0xF0, 0xF4
		db	0, 0, 0, 0xFF, 0, 0, 0
		db	0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20
		db	0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C, 0x40, 0x44
		db	0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60, 0x64
		db	0, 0, 0, 0, 0xFC, 0
		db	0x68, 0x6C, 0x70, 0x74, 0x78, 0x7C, 0x80, 0x84, 0x88
		db	0x8C, 0x90, 0x94, 0x98, 0x9C, 0xA0, 0xA4, 0xA8, 0xAC
		db	0xB0, 0xB4, 0xB8, 0xBC, 0xC0, 0xC4, 0xC8, 0xCC

;; The file size is padded out to exactly 256 bytes. (Note that the
;; last few entries of the table extend past this point into the
;; bss section of the program's memory image.)

ALIGN 256, db 0

;; The input buffer and output buffer are located here.

buffer equ $$ + 0x0200
