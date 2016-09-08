;; true.asm: Copyright (C) 2001 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o true true.asm && chmod +x true
;;	ln true false

BITS 32

		org	0x25490000

		db	0x7F, "ELF"
		dd	1
		dd	0
		dd	$$
		dw	2
		dw	3
		dd	_start
		dw	_start - $$
_start:		dec	ecx			; set ecx to a large value
		and	eax, dword 4
		pop	ebx			; remove argc, set ebx to 1
		pop	edi			; get argv[0]
		repnz scasb			; find end of argv[0]
		inc	eax			; 1 == exit system call
		and	bl, [byte edi - 5]	; 't' & 1 == 0, 'a' & 1 == 1
		int	0x80			; exit(bl)
		dw	0x20
		db	1

;; This is how the file looks when it is read as an (incomplete) ELF
;; header, beginning at offset 0:
;;
;; e_ident:	db	0x7F, "ELF"			; required
;;		db	1				; 1 = ELFCLASS32
;;		db	0				; (garbage)
;;		db	0				; (garbage)
;;		db	0				; (garbage)
;;		db	0x00, 0x00, 0x00, 0x00		; (unused)
;;		db	0x00, 0x00, 0x5F, 0x25
;; e_type:	dw	2				; 2 = ET_EXE
;; e_machine:	dw	3				; 3 = EM_386
;; e_version:	dd	0x2549001A			; (garbage)
;; e_entry:	dd	0x2549001A			; program starts here
;; e_phoff:	dd	4				; phdrs located here
;; e_shoff:	dd	0xAEF25F5B			; (garbage)
;; e_flags:	dd	0xFB5F2240			; (unused)
;; e_ehsize:	dw	0x80CD				; (garbage)
;; e_phentsize:	dw	0x20				; phdr entry size
;; e_phnum:	db	1				; one phdr in the table
;; e_shentsize:
;; e_shnum:
;; e_shstrndx:
;;
;; This is how the file looks when it is read as a program header
;; table, beginning at offset 4:
;;
;; p_type:	dd	1				; 1 = PT_LOAD
;; p_offset:	dd	0				; read from top of file
;; p_vaddr:	dd	0x25490000			; load at this address
;; p_paddr:	dd	0x00030002			; (unused)
;; p_filesz:	dd	0x2549001A			; too big, but ok
;; p_memsz:	dd	0x2549001A			; equal to file size
;; p_flags:	dd	4				; 4 = PF_R
;; p_align:	dd	0xAEF25F5B			; (garbage)
;;
;; Note that the top two bytes of the file's origin (0x49 0x25)
;; correspond to the instructions "dec ecx" and the first byte of "and
;; eax, IMM".
;;
;; The fields marked as unused are either specifically documented as
;; not being used, or not being used with 386-based implementations.
;; Some of the fields marked as containing garbage are not used when
;; loading and executing programs. Other fields containing garbage are
;; accepted because Linux currently doesn't examine then.
