;; factor.asm: Copyright (C) 1999 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o factor factor.asm && chmod +x factor
;;
;; Usage: factor [NUMBER]...    Print the prime factors of each NUMBER.
;;        factor --help         Display online help and exit.
;;        factor --version      Display version information and exit.
;;
;; Valid numbers are integers between 0 and 2^64 - 1 inclusive. If no
;; arguments are specified on the command line, numbers are read from
;; standard input.
;;
;; This is version 1.3 of factor.asm, written and released 2009.
;; Version 1.2 of factor.asm was written and released 2001.
;; Version 1.0 of factor.asm was written and released 1999.

BITS 32

;; Some magic numbers.

%define	stdout		1
%define	stderr		2

%define bias		0x3FFF		; bias of the floating-point exponent

;; The program's data. This structure appears just past the file's
;; image, in zero-initialized memory.

%define	iobuf_size	80

STRUC data
factor:		resd	2		; number being tested for factorhood
getchar_rel:	resd	1		; address of getchar() function
write_rel:	resd	1		; address of write() function
exit_rel:	resd	1		; address of exit() function
exitcode:	resd	1		; the program's exit code
		resd	2		; (filler for 32-byte alignment)
buf:		resd	4		; general floating-point buffer
iobuf:		resb	iobuf_size	; buffer for text input and output
ENDSTRUC


;;
;; The ELF executable file structures
;;
;; The following structures provide the minimum necessary data
;; required by the ELF standard in order to dynamically link with
;; libc and obtain pointers to the three standard functions that this
;; program needs to call. Wherever possible, the ELF structures have
;; been made to overlap with each other to reduce overall size. The
;; various magic numbers are identified in comments.
;;

		org	0x08048000

;; A basic ELF header, indicating that the program segment header
;; has three entries, and that there is no section header table.

		db	0x7F, 'ELF'
		db	1			; ELFCLASS32
		db	1			; ELFDATA2LSB
		db	1			; EV_CURRENT
		db	0			; ELFOSABI_NONE
		dd	0
		dd	0
		dw	2			; ET_EXEC
		dw	3			; EM_386
		dd	1			; EV_CURRENT
		dd	_start
		dd	phdrs - $$
		dd	0
		dd	0
		dw	0x34			; sizeof(Elf32_Ehdr)
		dw	0x20			; sizeof(Elf32_Phdr)
		dw	3
		dw	0
		dw	0
		dw	0

;; The hash table. With only four symbols to hash, this structure is
;; purely a formality. The last two entries in the hash table, a
;; 1 and a 0, overlap with the next structure.

hash:
		dd	1
		dd	5
		dd	3
		dd	0, 2, 4

;; The program header table contains three entries. The first one
;; indicates that the entire file is to be loaded into read-write-exec
;; memory. The second entry identifies the location of the _DYNAMIC
;; section, and the third entry gives the location of the interpreter
;; pathname. The last dword value of the table, a 1, overlaps with the
;; next structure.

phdrs:
		dd	1			; PT_LOAD
		dd	0
		dd	$$
		dd	$$
		dd	file_size
		dd	mem_size
		dd	7			; PF_R | PF_W | PF_X
		dd	0x1000
		dd	2			; PT_DYNAMIC
		dd	dynamic - $$
		dd	dynamic
		dd	dynamic
		dd	dynamic_size
		dd	dynamic_size
		dd	6			; PF_R | PF_W
		dd	4
		dd	3			; PT_INTERP
		dd	interp - $$
		dd	interp
		dd	interp
		dd	interp_size
		dd	interp_size
		dd	4			; PF_R

;; The _DYNAMIC section. Indicates the presence and location of the
;; dynamic symbol section (and associated string table and hash table)
;; and the relocation section. The final DT_NULL entry in the dynamic
;; section overlaps with the next structure.

dynamic:
		dd	1,  libc_name		; DT_NEEDED
		dd	4,  hash		; DT_HASH
		dd	5,  dynstr		; DT_STRTAB
		dd	6,  dynsym		; DT_SYMTAB
		dd	10, dynstr_size		; DT_STRSZ
		dd	11, 0x10		; DT_SYMENT
		dd	17, reltext		; DT_REL
		dd	18, reltext_size	; DT_RELSZ
		dd	19, 0x08		; DT_RELENT
dynamic_size equ $ - dynamic + 8

;; The dynamic symbol table. Entries are included for the _DYNAMIC
;; section and the three functions imported from libc: getchar(),
;; write(), and exit().

dynsym:
		dd	0
		dd	0
		dd	0
		dw	0
		dw	0
dynamic_sym equ 1
		dd	dynamic_name
		dd	dynamic
		dd	0
		dw	0x11			; STB_GLOBAL, STT_OBJECT
		dw	0xFFF1			; SHN_ABS
exit_sym equ 2
		dd	exit_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
getchar_sym equ 3
		dd	getchar_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
write_sym equ 4
		dd	write_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0

;; The relocation table. The addresses of the three functions imported
;; from libc are stored in the program's data area. Since they will be
;; used with indirect calls, the simple R_386_32 relocation is used.

reltext:
		dd	dataorg + write_rel
		db	1			; R_386_32
		db	write_sym
		dw	0
		dd	dataorg + getchar_rel
		db	1			; R_386_32
		db	getchar_sym
		dw	0
		dd	dataorg + exit_rel
		db	1			; R_386_32
		db	exit_sym
		dw	0
reltext_size equ $ - reltext

;; The interpreter pathname. The final NUL byte appears in the next
;; section.

interp:
		db	'/lib/ld-linux.so.2'
interp_size equ $ - interp + 1

;; The string table for the dynamic symbol table.

dynstr:
		db	0
libc_name equ $ - dynstr
		db	'libc.so.6', 0
dynamic_name equ $ - dynstr
		db	'_DYNAMIC', 0
exit_name equ $ - dynstr
		db	'exit', 0
getchar_name equ $ - dynstr
		db	'getchar', 0
write_name equ $ - dynstr
		db	'write', 0
dynstr_size equ $ - dynstr


;;
;; The program proper
;;
;; A number that is to be factored can be as high as 2^64 - 1.
;; Therefore, it can have no more than 63 prime factors, and at most
;; one prime factor greater than 2^32 - 1.
;;
;; The program works by testing the number against all possible
;; divisors except multiples of 2, 3, and 5 (the actual numbers 2, 3,
;; and 5 are tested directly before entering the main loop) until:
;;
;; 1. a factor is found that divides the number evenly, or
;; 2. the integer portion of a quotient exceeds the divisor, or
;; 3. the divisor exceeds 2^32 - 1.
;;
;; In the first case, the program outputs the new factor, the division
;; currently being computed is discarded, and the integer quotient
;; becomes the new number to test against for future divisions. In the
;; other two cases, the program now knows that number currently being
;; tested is prime, and is itself output as the final factor.
;;
;; Although a number can have at most a dozen or so factors beyond 2,
;; 3, and 5 (and the vast majority of numbers will only have a few),
;; the program might have to execute over a billion divisions in order
;; to identify them. Therefore it is necessary to optimize for testing
;; unsuccessful candidate factors.
;;
;; The essential idea behind this program is to use the FPU for doing
;; the divisions, even though we are only interested in integer
;; values. This allows the ALU to be used in parallel while the
;; lengthy division instruction is running, ideally having the next
;; division queued up and ready to go just as the previous division is
;; completed. In order to do this, the results of each division need
;; to be evaluated without involving the FPU. Quotients are therefore
;; examined via bit-twiddling, in order to determine if a quotient is
;; an integer and/or if it exceeds the divisor.
;;
;; Only the full-sized 80-bit floating-point numbers has a full 64
;; bits of mantissa, so the ALU must examine the quotient in this form
;; in order to avoid losing needed precision. (The fact that 64 bits
;; is always sufficient to tell if a quotient is integral when the
;; divisor has 32 or fewer bits is left as an exercise to the reader.)
;;

;; The factorconst subroutine, called by factorize, repeatedly divides
;; the number at the top of the floating-point stack by the integer
;; stored in buf as long as the number continues to divide evenly. For
;; each successful division, the number is also displayed.

factorconst:						; num
		fild	qword [ebx]			; fact num
.loop:		fld	st1				; num  fact num
		fdiv	st1				; quot fact num
		fld	st0				; quot quot fact num
		frndint					; iquo quot fact num
		fcomp	st1				; quot fact num
		fnstsw	ax
		sahf
		jnz	short .return
		fstp	st2				; fact quot
		call	ebp
		jmp	short .loop
.return:	fcompp
		ret

;; factorize is the main subroutine of the program. It is called with
;; esi pointing to an NUL-terminated string representing the number to
;; factor. factorize finds and displays the number's prime factors. If
;; the number is invalid, an error message is output instead and
;; exitcode is set to 1.

factorize:

;; The first step is to translate the string into a number. 10.0 and 0.0
;; are pushed onto the floating-point stack.

		xor	eax, eax
		fild	word [byte ebx + ten - dataorg]	; 10.0
		fldz					; num  10.0

;; Each character in the string is checked; if it is not in the range
;; of '0' to '9' inclusive, the subroutine aborts. Otherwise, the top
;; of the stack is multiplied by ten and the value of the digit is
;; added to the product. The loop exits when a NUL byte is found.

.atoiloop:
		lodsb
		or	al, al
		jz	short .atoiloopexit
		fmul	st0, st1			; 10n  10.0
		sub	al, '0'
		jc	short .badinput
		cmp	al, 10
		jnc	short .badinput
		mov	[ebx], eax
		fiadd	dword [ebx]			; num  10.0
		jmp	short .atoiloop

;; If the string is not a valid number, a stock error message is
;; displayed to the user and the subroutine returns directly.

.badinput:
		mov	byte [byte ebx + exitcode], 1
		push	byte errmsgbadnum_size
		lea	eax, [byte ebx + errmsgbadnum - dataorg]
		push	eax
		push	byte stderr
		jmp	short .writeandret

;; The number's exponent is examined, and if the number is 2^64 or
;; greater, it is treated as invalid.

.atoiloopexit:
		fld	st0				; num  num  junk
		fstp	tword [byte ebx + buf]		; num  junk
		cmp	word [byte ebx + buf + 8], bias + 64
		jae	short .badinput

;; The number, having been validated, is displayed on standard output.
;; If the number is zero, no factoring should be done and the
;; subroutine exits early.
							; num  junk
		xor	edi, edi
		call	ebp
		ftst
		fnstsw	ax
		sahf
		jz	short .earlyout
		fstp	st1				; num

;; The factorconst subroutine is called three times, with the factor
;; set to two, three, and five, respectively.

		mov	edi, factorconst
		mov	byte [ebx], 2
		call	edi
		inc	byte [ebx]
		call	edi
		mov	byte [ebx], 5
		call	edi

;; If the number is now equal to one, the subroutine is finished and
;; exits immediately.

		fld1					; 1.0  num
		fcom	st1
		fnstsw	ax
		sahf
		jz	short .earlyout

;; factor is initialized to 7, and edi is initialized with a sequence
;; of eight four-bit values that represent the cycle of differences
;; between subsequent integers not divisible by 2, 3, or 5. A division
;; by 7 is begun, and the main loop is entered.
							; junk num
		mov	byte [ebx], 7
		fild	qword [ebx]			; fact junk num
		fdivr	st0, st2			; quot junk num
		mov	edi, 0x42424626

;; The main loop of the factorize subroutine. The current factor to
;; evaluate (for which the division operation should just be finishing
;; up) is saved in esi. edi is advanced to the next step value, and
;; factor is incremented. If it overflows, then all possible 32-bit
;; factors have been exhausted.

.mainloop:
		mov	esi, [ebx]
		rol	edi, 4
		mov	eax, edi
		and	eax, byte 0x0F
		add	[ebx], eax
		jc	short .lastfactor

;; The quotient from the division by the current potential factor is
;; stored in the buffer, and division by the factor for the next
;; iteration is begun.
							; quot junk num
		fst	st1				; quot quot num
		fstp	tword [byte ebx + buf]		; quot num
		fild	qword [ebx]			; fact quot num
		fdivr	st0, st2			; nquo quot num

;; The integer portion of the quotient is isolated and tested against
;; the divisor (i.e., the potential factor). If the quotient is
;; smaller, then the loop has passed the number's square root.

		mov	edx, [byte ebx + buf + 4]
		mov	ecx, bias + 31
		sub	ecx, [byte ebx + buf + 8]
		jc	short .keepgoing
		mov	eax, edx
		shr	eax, cl
		cmp	eax, esi
		jnc	short .keepgoing

;; Here, the program has determined that the number being tested has
;; no more factors. The number is displayed, along with a newline,
;; the FPU stack is cleared, and the subroutine ends.

.lastfactor:						; junk junk num
		fxch	st2				; num  junk junk
		call	ebp
		fstp	st0				; junk junk
.earlyout:	push	byte 1
		lea	esi, [byte ebx + ten - dataorg]
.writetostdout:	push	esi
		push	byte stdout
.writeandret:	call	[byte ebx + write_rel]
		add	esp, byte 12
		fcompp
		ret

;; Otherwise, the integer portion of the quotient is shifted out. If
;; any nonzero bits are left, then the quotient is not an integer, and
;; thus our candidate is not a factor. The program then proceeds to
;; the next iteration.

.keepgoing:
		mov	eax, [byte ebx + buf]
		neg	ecx
		js	short .shift32
		xchg	eax, edx
		xor	eax, eax
.shift32:	shld	edx, eax, cl
		shl	eax, cl
		or	eax, edx
		jnz	short .mainloop

;; At this point, a new factor has been found. The number being
;; factored is therefore replaced with the quotient of the previous
;; division, and the result of the division in progress is junked. The
;; new factor is displayed, and the current factor is set back to esi
;; so that it can be tested again.
							; junk num  junk
		ffree	st2				; junk num
		mov	[ebx], esi
		ror	edi, 4
		fild	qword [ebx]			; fact junk num
		call	ebp
		fdivr	st0, st2			; quot junk num
		jmp	short .mainloop


;; itoa64 is the numeric output subroutine. When the subroutine is
;; called, the number to be displayed should be on the top of the
;; floating-point stack, and there should be no more than four other
;; numbers on the stack. A space is prefixed to the output, unless
;; edi is zero, in which case a colon is suffixed instead.

itoa64:

;; 10 is placed on the FPU stack. esi is set to point to the end of
;; the buffer that will hold the decimal representation, with ecx
;; pointing just past the end.
							; num  +
		fld	st0				; rnum num  +
		fild	word [byte ebx + ten - dataorg]	; 10.0 rnum num  +
		lea	ecx, [byte ebx + iobuf + 32]
		lea	esi, [byte ebx + iobuf + 31]

;; At each iteration, the number is reduced modulo 10. This remainder
;; is subtracted from the number (and stored in iobuf as an ASCII
;; digit). The difference is then divided by ten, and if the quotient
;; is zero the loop exits. Otherwise, the quotient replaces the number
;; for the next iteration.

.loop:
		fld	st1				; rnum 10.0 rnum num  +
		fprem					; rem  10.0 rnum num  +
		fist	word [ecx]
		fsubp	st2				; 10.0 10rn num  +
		fcom	st1
		fnstsw	ax
		fdiv	st1, st0			; 10.0 rnum num  +
		mov	al, [ecx]
		add	al, '0'
		mov	[esi], al
		dec	esi
		sahf
		jbe	short .loop

;; A space is added to the front and a colon is added to the end. The
;; value of edi will determine which one is included in the output.
;; write() is called, and the subroutine ends.

		mov	byte [esi], ' '
		mov	byte [ecx], ':'
		sub	ecx, esi
		or	edi, edi
		jnz	short .prefix
		inc	esi
.prefix:	push	ecx
		jmp	short factorize.writetostdout


;; Here is the program's entry point.

_start:

;; argc and argv[0] are removed from the stack. ebx is initialized to
;; point to the data, and ebp holds the address of the itoa64
;; subroutine. These registers, along with esi and edi, are guaranteed
;; to be preserved across calls to functions in libc.

		pop	eax
		pop	esi
		mov	ebx, dataorg
		mov	ebp, itoa64

;; If there are no further arguments, then the program proceeds to the
;; input loop. If argv[1] begins with a dash, then the program
;; branches to the option-parsing routine. Otherwise, the program
;; begins iterating through the command-line arguments.

		dec	eax
		jle	short .inputloop
		pop	esi
		cmp	byte [esi], '-'
		jz	short .handleoption

;; The factorize subroutine is called once for each command-line
;; argument. When there are no more arguments, the program exits.
;; The exitcode will be zero if no errors occurred.

.argloop:
		call	factorize
		pop	esi
		or	esi, esi
		jnz	short .argloop
.mainexit:	push	dword [byte ebx + exitcode]
		call	[byte ebx + exit_rel]

;; The input loop routine. esi is pointed to iobuf, and edi, the
;; input byte counter, is reset to zero.

.inputloop:
		lea	esi, [byte ebx + iobuf]
		xor	edi, edi

;; The program reads one character at a time. If the character input
;; is a non-graphic character, then the program either breaks out of
;; the loop (if iobuf currently contains anything), ignores the
;; character (if iobuf is empty and the character is a space or
;; control character), or exits (if the character is EOF). Otherwise,
;; the character input is appended to iobuf and the loop continues.

.incharloop:
		call	[byte ebx + getchar_rel]
		cmp	al, ' '
		jg	short .isgraphic
		or	edi, edi
		jnz	short .incharloopexit
		or	eax, eax
		jns	short .incharloop
		jmp	short .mainexit
.isgraphic:	mov	[esi + edi], al
		inc	edi
		cmp	edi, byte iobuf_size - 1
		jnz	short .incharloop
.incharloopexit:

;; A NUL is appended to the string obtained from standard input, the
;; factorize subroutine is called, and the program loops.

		mov	[esi + edi], bl
		call	factorize
		jmp	short .inputloop

;; The program comes here when an option is given on the command
;; line. The introductory dashes are skipped over, and the initial
;; letter of the option is retrieved.

.handleoption:
		xor	edx, edx
.dashes:	lodsb
		cmp	al, '-'
		jz	short .dashes

;; There are three possibilities: v(ersion), h(elp), or an invalid
;; option. Each one requires a different text to be displayed, with
;; the last one going to stderr instead of stdout.

		lea	ecx, [byte edx + versionmsg_size]
		lea	esi, [byte ebx + versionmsg - dataorg]
		cmp	al, 'v'
		jz	short .msgandexit
		mov	cl, helpmsg_size
		sub	esi, ecx
		cmp	al, 'h'
		jz	short .msgandexit
		mov	cl, errmsgbadopt_size
		sub	esi, ecx
		inc	edx

;; esi points to the text to display, ecx holds the text's length, and
;; edx contains an exit code of 0 or 1. The text is passed to write(),
;; with edx deciding whether stdout or stderr is used, and the program
;; exits.

.msgandexit:
		mov	[byte ebx + exitcode], edx
		push	ecx
		push	esi
		inc	edx
		push	edx
		call	[byte ebx + write_rel]
		jmp	short .mainexit


;;
;; Message texts.
;;

errmsgbadopt:	db	'Invalid option; try --help.', 10
errmsgbadopt_size equ $ - errmsgbadopt

helpmsg:	db	'factor [--help | --version | NUMBER...]', 10
		db	'Print the prime factors of each NUMBER.', 10
		db	'Read from stdin if no arguments.', 10
helpmsg_size equ $ - helpmsg

versionmsg:	db	'factor 1.3', 10
		db	'Copyright 2009 Brian Raiter. License GPLv2+.', 10
versionmsg_size equ $ - versionmsg

errmsgbadnum:	db	'Invalid number.'
ten:		db	10
errmsgbadnum_size equ $ - errmsgbadnum


;;
;; End of the file image.
;;

file_size equ $ - $$

;; The data structure's location is set at 1K from the origin. ebx
;; points to this throughout the program.

dataorg equ $$ + 0x0400

mem_size equ dataorg + data_size - $$
