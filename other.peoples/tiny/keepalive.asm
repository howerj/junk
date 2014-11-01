;; keepalive.asm: Copyright (C) 2001 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o keepalive keepalive.asm && chmod +x keepalive

BITS 32

		org	0x05426000

		db	0x7F, "ELF"
		dd	1
		dd	0
		dd	$$
		dw	2
		dw	3
		dd	_start
		db	_start - $$
_start:		pusha				; Save the current state
		inc	edx			; Set output length to one byte
		add	eax, dword 4		; write system call number
		mov	ecx, esp		; Point ecx at a buffer
		push	ecx			; Save buffer pointer
		mov	[ecx], word 0x0107	; 263 seconds, or an ASCII BEL
		inc	ebx			; stdout file descriptor
		cmp	eax, 0x00010020
		int	0x80			; 1. Lather the bell
		pop	ebx			; Point ebx at timespec
		mov	al, 162			; nanosleep system call number
		int	0x80			; 2. Rinse for 263 seconds
		popa				; Restore the saved state
		jmp	short _start		; 3. Repeat

;; This is how the file looks when it is read as an ELF header,
;; beginning at offset 0:
;;
;; e_ident:	db	0x7F, "ELF"			; required
;;		db	1				; 1 = ELFCLASS32
;;		db	0				; (garbage)
;;		db	0				; (garbage)
;;		db	0				; (garbage)
;;		db	0x00, 0x00, 0x00, 0x00		; (unused)
;;		db	0x00, 0x60, 0x42, 0x05
;; e_type:	dw	2				; 2 = ET_EXE
;; e_machine:	dw	3				; 3 = EM_386
;; e_version:	dd	0x05426019			; (garbage)
;; e_entry:	dd	0x05426019			; program starts here
;; e_phoff:	dd	4				; phdrs located here
;; e_shoff:	dd	0x6651E189			; (garbage)
;; e_flags:	dd	0x000701C7			; (unused)
;; e_ehsize:	dw	0x3D43				; (garbage)
;; e_phentsize:	dw	0x20				; phdr entry size
;; e_phnum:	db	1				; one phdr in the table
;; e_shentsize:	db	0x80CD				; (garbage)
;; e_shnum:	db	0xB05B				; (garbage)
;; e_shstrndx:	db	0xCDA2				; (garbage)
;;
;; This is how the file looks when it is read as a program header
;; table, beginning at offset 4:
;;
;; p_type:	dd	1				; 1 = PT_LOAD
;; p_offset:	dd	0				; read from top of file
;; p_vaddr:	dd	0x05426000			; load at this address
;; p_paddr:	dd	0x00030002			; (unused)
;; p_filesz:	dd	0x05426019			; too big, but ok
;; p_memsz:	dd	0x05426019			; equal to file size
;; p_flags:	dd	4				; 4 = PF_R
;; p_align:	dd	0x6651E189			; (garbage)
;;
;; Note that the top three bytes of the file's origin (0x60 0x42 0x05)
;; correspond to the instructions "pusha", "inc edx", and the first
;; byte of "add eax, IMM".
;;
;; The fields marked as unused are either specifically documented as
;; not being used, or not being used with 386-based implementations.
;; Some of the fields marked as containing garbage are not used when
;; loading and executing programs. Other fields containing garbage are
;; accepted because Linux currently doesn't examine then.
