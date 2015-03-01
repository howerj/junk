;; puzzle.asm: Copyright (C) 2011 Brian Raiter <breadbox@muppetlabs.com>
;; Licensed under the terms of the GNU General Public License, either
;; version 2 or (at your option) any later version.
;;
;; To build:
;;	nasm -f bin -o puzzle puzzle.asm && chmod +x puzzle

BITS 32

;; Definitions that would have been supplied by standard header files,
;; if only there existed assembler-friendly versions of them.

;; Definitions from X.h.

%define KeyPress		2
%define ButtonPress		4
%define Expose			12
%define ClientMessage		33

%define KeyPressMask		(1 << 0)
%define ButtonPressMask		(1 << 2)
%define ExposureMask		(1 << 15)

%define Button1			1

%define FillSolid		0
%define FillStippled		2

;; Definitions from Xlib.h.

%define RootWindowOfScreen(s)	dword [s + 8]
%define DefaultDepthOfScreen(s)	dword [s + 36]
%define DefaultGCOfScreen(s)	dword [s + 44]
%define WhitePixelOfScreen(s)	dword [s + 52]
%define BlackPixelOfScreen(s)	dword [s + 56]

%define XOfXButtonEvent(e)	dword [e + 32]
%define YOfXButtonEvent(e)	dword [e + 36]
%define ButtonOfXButtonEvent(e)	dword [e + 52]

%macro DefaultScreenOfDisplay 2
	mov	%1, dword [%2 + 140]
	add	%1, dword [%2 + 132]
%endmacro

;; Constants used by the program.

digitheight	equ	11			; height of digit in pixels
tilesize	equ	19			; height & width of each tile
tilerowbytes	equ	(tilesize + 7) >> 3	; width of tile in bytes
framesize	equ	5			; height & width of frame
windowsize	equ	(framesize * 2 + tilesize * 4 + 3) ; total window size
myeventmask	equ	(ExposureMask | ButtonPressMask)   ; events to receive

;; Macro for accessing addresses in .bss and some of .text via ebp.

%define	ADDR(a)	ebp + ((a) - _ebp_pos_)

;;
;; The ELF executable file structures
;;
;; The following provide the structures required by the ELF standard
;; in order to dynamically link with libX11.so and obtain pointers to
;; the various functions that this program needs to call. Where
;; possible, the ELF structures have been made to overlap with each
;; other. Comments identify various magic numbers.
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

;; A degenerate hash table. This structure is purely a formality. The
;; last 5 entries in the hash table (11, 16, 1, 10, 4) overlap
;; with the next structure.

hash:
		dd	1			; nbucket = 1
		dd	dynsym_count		; nchain = 18
		dd	15
		dd	0, 2, 3, 17, 5, 6, 7	; a chain with 18 links
		dd	8, 9, 16, 13, 12, 0

;; The _DYNAMIC section. Indicates the presence and location of the
;; dynamic symbol section (and associated string table and hash table)
;; and the relocation section. The final DT_NULL entry in the dynamic
;; section overlaps with the next structure.

dynamic:
		dd	11, 16			; DT_SYMENT = sizeof(Elf32_Sym)
		dd	1,  libX11_name		; DT_NEEDED = 10
		dd	4,  hash		; DT_HASH
		dd	5,  dynstr		; DT_STRTAB
		dd	6,  dynsym		; DT_SYMTAB
		dd	10, dynstr_size		; DT_STRSZ
		dd	17, reltext		; DT_REL
		dd	18, reltext_size	; DT_RELSZ
		dd	19, 8			; DT_RELENT = sizeof(Elf32_Rel)
dynamic_size equ $ - dynamic + 8

;; The dynamic symbol table. Entries are included for the _DYNAMIC
;; section and the seventeen functions imported from libX11.so.

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
XClearArea_sym equ 2
		dd	XClearArea_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XCopyArea_sym equ 3
		dd	XCopyArea_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XCreateBitmapFromData_sym equ 4
		dd	XCreateBitmapFromData_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XCreatePixmapFromBitmapData_sym equ 5
		dd	XCreatePixmapFromBitmapData_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XCreateSimpleWindow_sym equ 6
		dd	XCreateSimpleWindow_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XFillRectangle_sym equ 7
		dd	XFillRectangle_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XInternAtom_sym equ 8
		dd	XInternAtom_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XMapWindow_sym equ 9
		dd	XMapWindow_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XNextEvent_sym equ 10
		dd	XNextEvent_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XOpenDisplay_sym equ 11
		dd	XOpenDisplay_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XSelectInput_sym equ 12
		dd	XSelectInput_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XSetFillStyle_sym equ 13
		dd	XSetFillStyle_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XSetStipple_sym equ 14
		dd	XSetStipple_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XSetWMName_sym equ 15
		dd	XSetWMName_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XSetWMProtocols_sym equ 16
		dd	XSetWMProtocols_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
XStringListToTextProperty_sym equ 17
		dd	XStringListToTextProperty_name
		dd	0
		dd	0
		dw	0x12			; STB_GLOBAL, STT_FUNC
		dw	0
dynsym_count equ 18

;; The relocation table. The addresses of the functions imported from
;; libX11 are stored in the program's bss area. Since they will be
;; used with indirect calls, the R_386_32 relocation is used throughout.

reltext:
		dd	XClearArea_rel
		db	1, XClearArea_sym, 0, 0
		dd	XCopyArea_rel
		db	1, XCopyArea_sym, 0, 0
		dd	XCreateBitmapFromData_rel
		db	1, XCreateBitmapFromData_sym, 0, 0
		dd	XCreatePixmapFromBitmapData_rel
		db	1, XCreatePixmapFromBitmapData_sym, 0, 0
		dd	XCreateSimpleWindow_rel
		db	1, XCreateSimpleWindow_sym, 0, 0
		dd	XFillRectangle_rel
		db	1, XFillRectangle_sym, 0, 0
		dd	XInternAtom_rel
		db	1, XInternAtom_sym, 0, 0
		dd	XMapWindow_rel
		db	1, XMapWindow_sym, 0, 0
		dd	XNextEvent_rel
		db	1, XNextEvent_sym, 0, 0
		dd	XOpenDisplay_rel
		db	1, XOpenDisplay_sym, 0, 0
		dd	XSelectInput_rel
		db	1, XSelectInput_sym, 0, 0
		dd	XSetFillStyle_rel
		db	1, XSetFillStyle_sym, 0, 0
		dd	XSetStipple_rel
		db	1, XSetStipple_sym, 0, 0
		dd	XSetWMName_rel
		db	1, XSetWMName_sym, 0, 0
		dd	XSetWMProtocols_rel
		db	1, XSetWMProtocols_sym, 0, 0
		dd	XStringListToTextProperty_rel
		db	1, XStringListToTextProperty_sym, 0, 0
reltext_size equ $ - reltext

;; The interpreter pathname. The final NUL byte appears in the next
;; section.

interp:
		db	'/lib/ld-linux.so.2'
interp_size equ $ - interp + 1

;; The string table for the dynamic symbol table.

dynstr:
		db	0
dynamic_name equ $ - dynstr
		db	'_DYNAMIC', 0
libX11_name equ $ - dynstr
		db	'libX11.so.6', 0
XClearArea_name equ $ - dynstr
		db	'XClearArea', 0
XCopyArea_name equ $ - dynstr
		db	'XCopyArea', 0
XCreateBitmapFromData_name equ $ - dynstr
		db	'XCreateBitmapFromData', 0
XCreatePixmapFromBitmapData_name equ $ - dynstr
		db	'XCreatePixmapFromBitmapData', 0
XCreateSimpleWindow_name equ $ - dynstr
		db	'XCreateSimpleWindow', 0
XFillRectangle_name equ $ - dynstr
		db	'XFillRectangle', 0
XInternAtom_name equ $ - dynstr
		db	'XInternAtom', 0
XMapWindow_name equ $ - dynstr
		db	'XMapWindow', 0
XNextEvent_name equ $ - dynstr
		db	'XNextEvent', 0
XOpenDisplay_name equ $ - dynstr
		db	'XOpenDisplay', 0
XSelectInput_name equ $ - dynstr
		db	'XSelectInput', 0
XSetFillStyle_name equ $ - dynstr
		db	'XSetFillStyle', 0
XSetStipple_name equ $ - dynstr
		db	'XSetStipple', 0
XSetWMName_name equ $ - dynstr
		db	'XSetWMName', 0
XSetWMProtocols_name equ $ - dynstr
		db	'XSetWMProtocols', 0
XStringListToTextProperty_name equ $ - dynstr
		db	'XStringListToTextProperty', 0
dynstr_size equ $ - dynstr

;;
;; The program proper.
;;

;; The slidetile function moves the tile at position eax into the
;; currently empty position. The absolute difference between empty and
;; eax must be either 4 or 1, and in the latter case empty and eax
;; need to point to the same row. If the requested move is invalid,
;; no change is made and the zero flag will be cleared upon return.
;; Otherwise, both puzzle and empty are updated. eax and edx are
;; altered by this function.

slidetile:
		mov	edx, [ADDR(empty)]
		sub	edx, eax
		jns	.positive
		neg	edx
.positive:	cmp	edx, byte 4
		jz	.goodmove
		cmp	edx, byte 1
		jnz	.badmove
.checkrow:	mov	edx, [ADDR(empty)]
		xor	edx, eax
		and	edx, byte ~3
		jnz	.badmove
.goodmove:	cdq
		xchg	[ADDR(puzzle) + eax], dl
		xchg	eax, [ADDR(empty)]
		mov	[ADDR(puzzle) + eax], dl
.badmove:	ret

;; The program begins here. ebp is initialized to point to the middle
;; of the program's data, a value that it will keep throughout this
;; program. eax is cleared and a zero value is pushed on the stack
;; (to be used later by XOpenDisplay).

_start:
		mov	ebp, _ebp_pos_
		xor	eax, eax
		push	eax

;; This routine creates the bitmap data for the tile images. The tile
;; bitmap, when constructed, is a 19x323 (monochrome) bitmap, divided
;; up into 17 tile images, each one 19x19 in dimension. The topmost
;; tile image represents the empty space; the remaining 16 images
;; represent the actual tiles, numbered 1 through 16.
;;
;; The program contains pixel data for the ten digits. Each digit is
;; represented by an 8x11 bitmap, or 11 bytes. The program copies each
;; digit one row at a time into the destination tile.

;; esi is pointed to the pixel data for the digits, and edi is pointed
;; to the empty memory where the tile images bitmap will be
;; constructed. The digits zero through nine are loaded into eax, one
;; pixel row at a time, and copied to two separate tiles, with the
;; second copy also containing the pixel data for the one digit
;; included. This loop therefore actually creates numbered tiles for
;; zero through nineteen, inclusive, even though only one through
;; sixteen will be used.

		lea	esi, [ADDR(digitbits)]
		mov	edi, tilebits + 3 * tilerowbytes
		lea	ecx, [eax + 10]
.digitloop:	cdq
.digitrowloop:	xor	eax, eax
		lodsb
		shl	eax, 7
		stosw
		shl	eax, 2
		mov	al, [ADDR(digitbits) + digitheight + edx]
		shl	eax, 1
		mov	[edi + 10 * tilesize * tilerowbytes - 2], eax
		inc	edi
		inc	edx
		cmp	edx, byte digitheight
		jnz	.digitrowloop
		add	edi, byte (tilesize - digitheight) * tilerowbytes
		loop	.digitloop

;; Three of the seventeen tiles need to be manually fixed up after the
;; loop exits. Tile zero needs to have its digit overwritten with a
;; stipple-like pattern that represents the tile-less empty space. In
;; addition, tiles four and fourteen are both missing some pixels.
;; Unlike the other digits, which fit snugly in an 8x11 rectangle, the
;; images for the digit four extends into a ninth column. Rather than
;; store pixel data for a ninth column for all the digits, which would
;; double the size of the digit pixel data, the program ORs the
;; missing pixels into the two affected tiles manually.

		mov	eax, 0x04000004
		add	edi, byte (2 * tilesize + 3) * tilerowbytes
		or	[edi + (2 * tilesize + 3) * tilerowbytes + 2], eax
		or	[edi + (2 * tilesize + 3) * tilerowbytes + 8], al
		lea	esi, [ADDR(tilebits)]
		mov	dword [esi + tilerowbytes], 0x00044444
		mov	dword [esi + 3 * tilerowbytes], 0x00011111
		lea	edi, [esi + 4 * tilerowbytes]
		mov	cl, (tilesize - 4) * tilerowbytes
		rep movsb
		shl	eax, 5
		add	edi, byte (2 * tilesize + 3) * tilerowbytes
		or	[edi + (tilesize + 6) * tilerowbytes + 1], eax
		or	[edi + (tilesize + 6) * tilerowbytes + 7], al

;; With basic initialization completed, XOpenDisplay is called to
;; connect to the X server. If this call fails, the program quits
;; directly with a nonzero exit code. Otherwise, the Display pointer
;; value is saved in ebx, which is where it will be stored for the
;; majority of the program's lifespan.

		call	[ADDR(XOpenDisplay_rel)]
		or	eax, eax
		jnz	.connected
		inc	eax
		mov	ebx, eax
		int	0x80
.connected:	xchg	eax, ebx


;; Here the program stores a local copy of the current stack pointer,
;; to be restored inside the event loop.

		mov	[ADDR(stackptr)], esp

;; A pre-existing graphics context is obtained.

		lea	eax, [byte ebx + 80]
		DefaultScreenOfDisplay esi, (eax - 80)
		mov	eax, DefaultGCOfScreen(esi)
		mov	[ADDR(gc)], eax

;; XCreatePixmapFromBitmapData is used to turn our tile image bitmap
;; into a pixmap compatible with the X server.

		push	DefaultDepthOfScreen(esi)
		push	WhitePixelOfScreen(esi)
		push	BlackPixelOfScreen(esi)
		push	dword tilesize * 17
		push	byte tilesize
		push	tilebits
		push	RootWindowOfScreen(esi)
		push	ebx
		call	[ADDR(XCreatePixmapFromBitmapData_rel)]
		mov	[ADDR(tileimages)], eax

;; XCreateSimpleWindow is called, and the returned Window value is
;; stored in esi.

		xor	edx, edx
		push	WhitePixelOfScreen(esi)
		push	BlackPixelOfScreen(esi)
		push	edx
		push	byte windowsize
		push	byte windowsize
		push	edx
		push	edx
		push	RootWindowOfScreen(esi)
		push	ebx
		call	[ADDR(XCreateSimpleWindow_rel)]
		xchg	eax, esi

;; The atom for WM_DELETE_WINDOW is retrieved so that the program can
;; use XSetWMProtocols to request to be notified when the window
;; manager tries to close the window.

		lea	eax, [ADDR(delwinproto)]
		mov	[esp + 4], eax
		call	[ADDR(XInternAtom_rel)]
		pop	ebx
		lea	edi, [ADDR(textprop)]
		mov	[edi], eax
		push	byte 1
		push	edi
		push	esi
		push	ebx
		call	[ADDR(XSetWMProtocols_rel)]

;; A string property is created for the string "Puzzle", and calling
;; XSetWMName makes this our window's title.

		push	edi
		push	byte 1
		lea	eax, [ADDR(titlestrlist)]
		push	eax
		call	[ADDR(XStringListToTextProperty_rel)]
		push	edi
		push	esi
		push	ebx
		call	[ADDR(XSetWMName_rel)]

;; A stipple pattern is created and selected into the GC; this will be
;; used to fill in the frame surrounding the tiles that make up the
;; body of the puzzle.

		push	byte 4
		push	byte 4
		push	ebp
		push	esi
		push	ebx
		call	[ADDR(XCreateBitmapFromData_rel)]
		push	eax
		push	dword [ADDR(gc)]
		push	ebx
		call	[ADDR(XSetStipple_rel)]

;; The program requests to be notified of exposure events and mouse
;; button events, and then finally XMapWindow is called to make the
;; window visible.

		push	myeventmask
		push	esi
		push	ebx
		call	[ADDR(XSelectInput_rel)]
		call	[ADDR(XMapWindow_rel)]

;; The puzzle is initialized in its solved state. Now the program
;; needs to mix it up. A very simple pseudo-random number generator is
;; used (basically the old linear congruential generator), seeded with
;; the lower half of the TSC.
;;
;; The puzzle needs to have about 1000 random moves made in order to
;; be properly randomized, so that every tile has a roughly equal
;; chance of being in a given position. Because this code selects the
;; tile to move without regard to their proximity to current location
;; of the empty space, approximately four times out of five it will
;; request an invalid move. Thus the loop must be repeated 5000 times
;; to achieve a properly random initial state.

		rdtsc
		mov	ecx, 5000
.shuffleloop:	mov	edx, 1103515245
		mul	edx
		lea	eax, [eax + ecx*2]
		push	eax
		shr	eax, 28
		call	slidetile
		pop	eax
		loop	.shuffleloop

;; Finally, the program begins the event loop, where it will remain
;; until it is time to exit. Each time through the stack is reset, so
;; that the code is spared the necessity of repeatedly cleaning up
;; after every function call.

.runloop:
		mov	esp, [ADDR(stackptr)]

;; XNextEvent is used to wait until an event can be retrieved. The
;; program can get three types of events: A mouse button press, an
;; expose event, or a client message. The latter type only occurs when
;; the user has attempted to close our window, so in that case the
;; program immediately exits. The others types are routed to the
;; appropriate routine following.

		lea	edi, [ADDR(event)]
		push	edi
		push	ebx
		call	[ADDR(XNextEvent_rel)]
		mov	eax, [edi]
		sub	eax, byte Expose
		jz	.render
		sub	eax, byte ButtonPress - Expose
		jz	.mouse
		sub	eax, byte ClientMessage - ButtonPress
		jnz	.runloop
.exit:		mov	ebx, eax
		inc	eax
		int	0x80

;; The program comes here upon retrieving a mouse click.

.mouse:

;; The X and Y coordinates of the mouse are retrieved, and after
;; subtracting the frame width and dividing by the width of the tiles,
;; the values can be combined to find the position of the tile that
;; was clicked on. This value is then passed to the slidetile routine.

		mov	ecx, ButtonOfXButtonEvent(edi)
		loop	.runloop
		mov	cl, tilesize + 1
		mov	eax, YOfXButtonEvent(edi)
		sub	eax, byte framesize
		idiv	cl
		sar	ah, 7
		xchg	eax, edx
		mov	eax, XOfXButtonEvent(edi)
		sub	eax, byte framesize
		idiv	cl
		sar	ah, 7
		shl	edx, 2
		add	eax, edx
		call	slidetile

;; After every call to slidetile, the puzzle's state is examined to
;; see if it is currently in the winning configuration. If so, the
;; last position is changed from empty to the sixteenth tile (and the
;; empty variable is set to a value well outside of the puzzle). In
;; either case, the program then proceeds to the rendering routine.

		mov	cl, 15
.checkloop:	cmp	[ADDR(puzzle) + ecx - 1], cl
		jnz	.render
		loop	.checkloop
		mov	byte [ADDR(puzzle) + 15], 16
		mov	byte [ADDR(empty)], 127

;; The program comes here whenever the window needs to be redrawn.

.render:

;; The rendering routine begins by calling XFillRectangle twice in a
;; row. On the first call, the entire window is filled with the
;; stipple pattern for the frame. The rectangle is then shrunk by four
;; pixels in all directions, and then this rectangle is filled with
;; solid black. The end result is to create the frame and the
;; one-pixel black border separating it from the tiles.

		xor	eax, eax
		lea	ecx, [eax + windowsize]
		lea	edi, [eax + FillStippled]
.again:		push	ecx
		push	ecx
		push	eax
		push	eax
		mov	ecx, [ADDR(gc)]
		push	ecx
		push	esi
		push	ebx
		push	edi
		push	ecx
		push	ebx
		call	[ADDR(XSetFillStyle_rel)]
		add	esp, byte 12
		call	[ADDR(XFillRectangle_rel)]
		or	edi, edi
		jz	.proceed
		xor	edi, edi
		lea	ecx, [edi + windowsize - framesize - 3]
		lea	eax, [edi + framesize - 1]
		jmp	short .again
.proceed:

;; The black pixels now filling the tile area will be used to provide
;; the lines separating the individual tiles from each other. However,
;; these lines are interrupted from reaching the frame on the
;; right-hand and bottom edges. In order to clear these, the program
;; draws two white lines along those sides, just inside the frame.
;; The program uses two calls to XClearArea with very thin rectangles
;; as a fast way to draw white lines without having to change the
;; current foreground color.

		push	edi
		push	byte windowsize - (framesize + 1) * 2
		push	byte 1
		push	byte framesize + 1
		push	byte windowsize - (framesize + 1)
		push	esi
		push	ebx
		call	[ADDR(XClearArea_rel)]
		push	edi
		push	byte 1
		push	byte windowsize - (framesize + 1) * 2
		push	byte windowsize - (framesize + 1)
		push	byte framesize + 1
		push	esi
		push	ebx
		call	[ADDR(XClearArea_rel)]

;; Now the program sets up a stack frame that will be used to
;; repeatedly call XCopyArea, with the tile image pixmap as the source
;; and the window as the destination.

		push	eax
		push	eax
		push	byte tilesize
		push	byte tilesize
		push	eax
		push	edi
		push	dword [ADDR(gc)]
		push	esi
		push	dword [ADDR(tileimages)]
		push	ebx

;; The program now enters a nested loop in order to draw the sixteen
;; tiles that make up the body of the puzzle. esi holds in turn each
;; tile's x-coordinate, and edi holds the y-coordinate. ebx points to
;; the puzzle array, and retrieves the number representing the tile at
;; that position, which is used to find the proper tile to copy from
;; the tile image pixmap. These coordinates are inserted into the
;; previously created stack frame just before invoking XCopyArea on
;; each iteration of the inner loop.

		lea	ebx, [ADDR(puzzle)]
		lea	edi, [edi + framesize]
.ydrawloop:	mov	[esp + 36], edi
		push	byte framesize
		pop	esi
.xdrawloop:	mov	[esp + 32], esi
		movzx	eax, byte [ebx]
		inc	ebx
		mov	cl, tilesize
		mul	cl
		mov	[esp + 20], eax
		call	[ADDR(XCopyArea_rel)]
		add	esi, byte tilesize + 1
		cmp	esi, byte windowsize - framesize
		jl	.xdrawloop
		add	edi, byte tilesize + 1
		cmp	edi, byte windowsize - framesize
		jl	.ydrawloop

;; ebx and esi's values (the Display pointer and the Window,
;; respectively) are restored from the stack, and the program returns
;; to the top of the main event loop.

		pop	ebx
		pop	esi
		pop	esi
		jmp	.runloop

;;
;; Program data.
;;

;; The window title text.

titlestr:	db	'Puzzle', 0

;; The protocol string indicating when the user closes our window.

delwinproto:	db	'WM_DELETE_WINDOW', 0

;; The graphic data for the digits 0 through 9. Each digit has 11
;; bytes of data, with each byte representing one row of eight pixels.

digitbits:	db	0x3C,0x66,0xDB,0xBD,0xA5,0xA5,0xA5,0xBD,0xDB,0x66,0x3C
		db	0x38,0x2C,0x24,0x2C,0x28,0x28,0x28,0x28,0x28,0x28,0x38
		db	0x7C,0xC6,0xBB,0xAD,0xB7,0xD8,0x6C,0x36,0xFB,0x81,0xFF
		db	0xFF,0x81,0xDF,0x6C,0x64,0xDC,0xB0,0xB7,0xDD,0x63,0x3E
		db	0xE0,0xB0,0x98,0xAC,0xB6,0xBB,0xBD,0x01,0xBF,0xA0,0xE0
		db	0xFF,0x81,0xFD,0x7D,0xC1,0xBF,0xA0,0xA7,0xBD,0xC3,0x7E
		db	0x7C,0x46,0x7B,0x7D,0xC1,0xBD,0xA5,0xA5,0xBD,0xC3,0x7E
		db	0xFF,0x81,0xBF,0xD0,0x58,0x68,0x2C,0x34,0x14,0x14,0x1C
		db	0x7E,0xC3,0xBD,0xBD,0xC3,0xBD,0xA5,0xA5,0xBD,0xC3,0x7E
		db	0x7E,0xC3,0xBD,0xA5,0xA5,0xBD,0x83,0xBE,0xDE,0x62,0x3E

;; ebp is set to point here during initialization. From here most of
;; the data needed by the program is within 127 bytes.

_ebp_pos_:

;; The 4x4 stipple pattern for the frame.

framebits:	db	0x05, 0x0E, 0x05, 0x0B

ALIGN 4

;; A string-pointer list of length one.

titlestrlist:	dd	titlestr

;; The puzzle is represented as a simple array of 16 bytes, with 0
;; representing the empty tile-less tile, and the bytes 1 through 15
;; (or 16) representing each numbered tile. The tiles are arranged in
;; the array left-to-right, top-to-bottom. In addition, the variable
;; empty holds the current offset of the tile-less position.
;;
;; The initial state of the puzzle is the winning configuration: tiles
;; one through fifteen in sequence, with the empty space in the
;; bottom-right corner.

puzzle:		db	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0
empty:		db	15

;;
;; End of the file image.
;;

file_size equ $ - $$

;;
;; Beginning of the program's bss section.
;; 

ABSOLUTE $
		resb	3	; the rest of the empty variable dword

;; This section is used to hold pointers to all of the imported X
;; functions used by the program. Note, however, that since many of
;; these functions are only used during initialization, several of
;; these locations are subsequently reused to hold values needed
;; afterwards.

stackptr:
XOpenDisplay_rel:		resd	1
gc:
XDefaultGC_rel:			resd	1
XInternAtom_rel:		resd	1
XCreateBitmapFromData_rel:	resd	1
tileimages:
XCreatePixmapFromBitmapData_rel:resd	1
XCreateSimpleWindow_rel:	resd	1
XSelectInput_rel:		resd	1
XMapWindow_rel:			resd	1
XSetStipple_rel:		resd	1
XSetWMName_rel:			resd	1
XSetWMProtocols_rel:		resd	1
XStringListToTextProperty_rel:	resd	1
XNextEvent_rel:			resd	1
XClearArea_rel:			resd	1
XCopyArea_rel:			resd	1
XFillRectangle_rel:		resd	1
XSetFillStyle_rel:		resd	1

;; A temporary buffer. This is where the program builds up the tile
;; bitmap, and is subsequently reused briefly to store a text property
;; structure, and then again inside the event loop to hold the
;; retrieved events.

textprop:
event:
tilebits:	resb	20 * tilesize * tilerowbytes

mem_size equ $ - $$
