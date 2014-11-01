PROGRAMS = true false hello keepalive bf hexdump ls base64 factor puzzle

all: $(PROGRAMS)

% : %.asm
	nasm -f bin -w+orphan-labels -o $@ $< && chmod +x $@

true     : true.asm
hello    : hello.asm
keepalive: keepalive.asm
bf       : bf.asm
hexdump  : hexdump.asm
ls       : ls.asm
base64   : base64.asm
factor   : factor.asm
puzzle   : puzzle.asm

false: true
	ln $< $@

clean:
	rm -f $(PROGRAMS)
