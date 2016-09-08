
{T pprolog.p -- picoProlog interpreter }

{ Copyright (C) J. M. Spivey 1992 }

{ This is the `picoProlog' interpreter described in the book `An
  Introduction to Logic Programming through Prolog' by Michael Spivey
  (Prentice Hall, 1995).  Copyright is retained by the author, but
  permission is granted to copy and modify the program for any purpose
  other than direct commercial gain.

  The text of this program must be processed by the `ppp'
  macro processor before it can be compiled. }

program picoProlog(input, output);

{ tunable parameters }
const
  MAXSYMBOLS = 511;  { max no. of symbols }
  HASHFACTOR = 90;  { percent loading factor for hash table }
  MAXCHARS = 2048;  { max chars in symbols }
  MAXSTRING = 128;  { max string length }
  MAXARITY = 63;  { max arity of function, vars in clause }
  MEMSIZE = 1000000;  { size of |mem| array }
  GCLOW = 10000;  { call GC when this much space left }
  GCHIGH = 50000;  { GC must find this much space }

{ special character values }
  { end of string }
  { tab character }
  { newline character }
  { end of file }

{S Coding conventions }

{ We ignore Pascal's stupid rule that all global variables must be
  declared together at the start of the program; likewise all global
  types and all global constants.  Many Pascal compilers relax the
  rule to make large programs easier to read and write; but if your
  Pascal compiler enforces it, you know what to do, and a text
  editor is the tool for the job. }

{ Most Pascal compilers implement a `default' part in case
  statements.  The macro |default| should be defined as the text that
  comes between the ordinary cases and the default part.  If the
  default part is like an ordinary case, but labelled with a keyword
  (say `others'), then the definition of |default| should include the
  semicolon that separates it from the preceding case, like this:
  `; others:'.  If your Pascal doesn't have default parts for case
  statements, most of them can be deleted, since they are only calls
  to |bad_tag| put there for robustness.  The only other one (in
  |Scan|) will need a little more work. }


{ Some Pascal implementations buffer terminal output, but provide a
  special procedure to flush the buffer; the |flush_out| macro
  should be defined to call whatever procedure is necessary.  A call
  to |flush_out| follows each prompt for input from the terminal, and
  the progress messages from the garbage collector. }



{ Pascal's numeric labels make code that uses |goto| statements
  unnecessarily obscure, so we define a few macros that have
  meaningful names but expand to plain integers that can be used
  as labels. }






{ When something goes drastically wrong, picoProlog sometimes needs
  to stop immediately.  In standard Pascal, this is achieved by a
  non-local jump to the label |end_of_pp|, located at the end of the
  main program.  But some Pascal compilers don't allow non-local
  jumps; they often provide a |halt| procedure instead.  The macro
  |abort| should be defined to do whatever is needed. }
label 999;


{ Here are a few convenient abbreviations: }
  { increment a variable }
  { decrement a variable }
  { return from procedure }
  { empty statement }

{S Error handling }

{ These macros print an error message, then either arrange for
  execution of a goal to abandoned (by clearing the |run| flag), or
  abandon the whole run of picoProlog.  They use the |$0| feature to
  allow for a list of arguments.

  Errors during execution of a goal are reported by |exec_error|; it
  sets the |run| flag to false, so the main execution mechanism will
  stop execution before starting on another resolution step. }

var run: boolean;  { whether execution should continue }
  dflag: boolean;  { switch for debugging code }





{S String buffer }

{ The strings that are the names of function symbols, variables,
  etc. are saved in the array |charbuf|: each string is
  represented elsewhere by an index |k| into this array, and the
  characters of the string are |charbuf[k]|,
  |charbuf[k+1]|,~\dots, terminated by the character |ENDSTR|.
  |charptr| is the last occupied location in |charbuf|.

  In addition to these `permanent' strings, there are `temporary'
  strings put together for some short-term purpose.  These are
  kept in arrays of size |MAXSTRING|, and are also terminated by
  |ENDSTR|. }

type
  permstring = 1..MAXCHARS;
  tempstring = array [1..MAXSTRING] of char;

var
  charptr: 0..MAXCHARS;
  charbuf: array [1..MAXCHARS] of char;

{ |StringLength| -- length of a tempstring }
function StringLength(var s: tempstring): integer;
  var i: 0..MAXSTRING;
begin
  i := 0;
  while s[i+1] <> chr(0) do i := i+1;
  StringLength := i
end;

{ |SaveString| -- make a tempstring permanent }
function SaveString(var s: tempstring): permstring;
  var i: 0..MAXSTRING;
begin
  if charptr + StringLength(s) + 1 > MAXCHARS then
    begin writeln; writeln('Panic: ', 'out of string space'); halt end;
  SaveString := charptr+1; i := 0;
  repeat
    i := i+1; charptr := charptr+1; charbuf[charptr] := s[i]
  until s[i] = chr(0)
end;

{ |StringEqual| -- compare a tempstring to a permstring }
function StringEqual(var s1: tempstring; s2: permstring): boolean;
  var i: integer;
begin
  i := 1;
  while (s1[i] <> chr(0)) and (s1[i] = charbuf[s2+i-1]) do i := i+1;
  StringEqual := (s1[i] = charbuf[s2+i-1])
end;

{ |WriteString| -- print a permstring }
procedure WriteString(s: permstring);
  var i: 1..MAXCHARS;
begin
  i := s;
  while charbuf[i] <> chr(0) do
    begin write(charbuf[i]); i := i+1 end
end;

{S Representation of terms }

{ It is now time to give the details of how terms are
  represented.  Each `term' is an index into the |mem| array that
  points to a small block of contiguous words.  The first word
  indicates the number and layout of the words that follow. It packs
  together the size of the node, and an integer code that determines
  the kind of term: |FUNC| for a compound term, |INT| for an
  integer, and so on.  Macros |t_kind(t)| and |t_size(t)| extract
  these from the first word of a term |t|.  There is also a bit in
  the first word that is used by the garbage collector for marking.
  The second word of the node, |t_shift(t) = mem[t+1]| is also
  reserved for the garbage collector.

  The layout of the remaining elements of |mem| that make up the
  term depends on the |t_kind| field.  For a |FUNC| term, there is
  the function symbol |t_func(t)|, and a variable number of
  arguments, which may be referred to as |t_arg(t, 1)|, |t_arg(t, 2)|,
  \dots, |t_arg(t, n)| where |n| is the arity of |t_func(t)|.

  For an |INT| term, there is just the integer value |t_ival(t)|,
  and for a |CHRCTR| term there is the character value |t_cval(t)|,
  which is actually the code |ord(c)|.  |CELL| nodes represent
  variables and have a |t_val| field that points to the value. |REF|
  nodes are the numeric markers in program clauses that refer to a
  slot in the frame for a clause; the |t_index| field is the
  index of the slot. |UNDO| nodes do not represent terms at all,
  but items on the trail stack; they share some of the layout of
  terms, so that they can be treated the same by the garbage
  collector. }

type
  pointer = integer;  { index into |mem| array }
  { null pointer }

type term = pointer;

    { one of |FUNC|, |INT|, \dots }
    { size in words }
    { GC mark }
  
  
  
  { for use by gc }
  { compound term }
    { \quad function symbol }
    { \quad arguments (start from 1) }
  { integer }
    { \quad integer value }
  { character }
    { \quad character value }
  { variable cell }
    { \quad value or |NULL| if unbound }
  { variable reference }
    { \quad index in frame }
  { trail item }
  { see later }
  { \dots\ plus no. of args }

{S Memory allocation }

{ Storage for most things is allocated from the big array |mem|.
  This array is in three parts: the heap and local stack, which grow
  upwards from the bottom of |mem|, and the global stack, which
  grows downwards from the top of |mem|.

  The heap stores the clauses that make up the program and running
  goal; it grows only while clauses are being input and not during
  execution, so there is no need for free space between the heap and
  local stack.  Program clauses become a permanent part of the heap,
  but goal clauses (and clauses that contain errors) can be
  discarded; so there is an extra variable |hmark| that indicates
  the beginning of the present clause.

  The local stack holds activation records for clauses during
  execution of goals, and the global stack other longer-lived data
  structures.  Both stacks expand and contract during execution of
  goals.  Also, there is a garbage collector that can reclaim
  inaccessible portions of the global stack. }

var
  lsp, gsp, hp, hmark: pointer;
  mem: array [1..MEMSIZE] of integer;

{ |LocAlloc| -- allocate space on local stack }
function LocAlloc(size: integer): pointer;
begin
  if lsp + size >= gsp then begin writeln; writeln('Panic: ', 'out of stack space'); halt end;
  LocAlloc := lsp + 1; lsp := lsp + size
end;

{ |GloAlloc| -- allocate space on global stack }
function GloAlloc(kind, size: integer): pointer;
  var p: pointer;
begin
  if gsp - size <= lsp then
    begin writeln; writeln('Panic: ', 'out of stack space'); halt end;
  gsp := gsp - size; p := gsp;
  mem[p] := 256 * kind + size;
  GloAlloc := p
end;

{ |HeapAlloc| -- allocate space on heap }
function HeapAlloc(size: integer): pointer;
begin
  if hp + size > MEMSIZE then begin writeln; writeln('Panic: ', 'out of heap space'); halt end;
  HeapAlloc := hp + 1; hp := hp + size
end;

  { test if a pointer is in the heap }
  { test if it is in the global stack }

{S Character input }

{ Pascal's I/O facilities view text files as sequences of lines,
  but it is more convenient for picoProlog to deal with a uniform
  sequence of characters, with the end of a line indicated by an
  |ENDLINE| character, and the end of a file by an |ENDFILE|
  character.  The routines here perform the translation (probably
  reversing a translation done by the Pascal run-time library).
  They also allow a single character to be `pushed back' on the
  input, so that the scanner can avoid reading too far. }

var
  interacting: boolean;  { whether input is from terminal }
  pbchar: char;  { pushed-back char, else |ENDFILE| }
  infile: text;  { the current input file }
  lineno: integer;  { line number in current file }
  filename: permstring;  { name of current file }

{ |FGetChar| -- get a character from a file }
function FGetChar(var f: text): char;
  var ch: char;
begin
  if eof(f) then
    FGetChar := chr(127)
  else if eoln(f) then
    begin readln(f); lineno := lineno+1; FGetChar := chr(10) end
  else
    begin read(f, ch); FGetChar := ch end
end;

{ |GetChar| -- get a character }
function GetChar: char;
begin
  if pbchar <> chr(127) then
    begin GetChar := pbchar; pbchar := chr(127) end
  else if interacting then
    GetChar := FGetChar(input)
  else
    GetChar := FGetChar(infile)
end;

{ |PushBack| -- push back a character on the input }
procedure PushBack(ch: char);
begin
  pbchar := ch
end;

{S Representation of clauses }

{ Clauses in the picoProlog program (and goals to be executed) have
  head and body literals in which the variables are replaced by |REF|
  nodes.  The clause itself is a segment of |mem| that has some
  fields at fixed offsets, followed by a variable-length sequence of
  pointers to the literals in the body of the clause, terminated by
  |NULL|.  Goal clauses have the same representation, but with |head
  = NULL|.  Macros |c_rhs| and |c_body| are defined so that
  |c_rhs(c)| is a pointer to the beginning of the sequence of
  pointers that makes up the clause body, and |c_body(c, i)| is the
  |i|'th literal in the body itself.

  Partially executed clause bodies are represented in the execution
  mechanism by the address of the pointer |p| to the first unsolved
  literal.  For cleanliness, we provide macros |g_first(p)| and
  |g_rest(p)| that respectively return the first literal itself, and
  a pointer that represents the remaining literals after the first
  one.  The test for the empty list is |g_first(p) = NULL|.

  The number of clauses tried against a goal literal is reduced by
  using associating each literal with a `key', calculated so that
  unifiable literals have matching keys. }

type clause = pointer;
  { no. of variables }
  { unification key }
  { next clause for same relation }
  { clause head }
  { clause body (ends with NULL) }

  { ... plus size of body + 1 }

  { first of a list of literals }
  { rest of the list }

{S Stack frames and interpreter registers }

{ The local stack is organized as a sequence of frames, each
  corresponding to an active copy of a program clause.  Most fields
  in a frame are copies of the values of the interpreter's
  `registers' when it was created, so here also is the declaration
  of those global registers.  The |tp| register that points to the
  top of the trail stack is declared later.

  The last part of a frame is a variable-length array of cells,
  containing the actual variables for the clause being used in the
  frame. The variables are numbered from 1, and each cell is of
  length |TERM_SIZE|, so the |f_local| macro contains the right
  formula so that |f_local(f, i)| is a pointer to the |i|'th cell. }

type frame = pointer;
  { the goal }
  { parent frame }
  { untried clauses }
  { previous choice-point }
  { global stack at creation }
  { trail state at creation }
  { no. of local variables }

  { \dots plus space for local variables }

{ |frame_size| -- compute size of a frame with |n| variables }


var
  current: pointer;  { current goal }
  call: term;  { |Deref|'ed first literal of goal }
  goalframe: frame;  { current stack frame }
  choice: frame;  { last choice point }
  base: frame;  { frame for original goal }
  proc: clause;  { clauses left to try on current goal }

{ |Deref| is a function that resolves the indirection in the
  representation of terms.  It looks up references in the frame, and
  following the chain of pointers from variable cells to their
  values.  The result is an explicit representation of the argument
  term; if the frame is non-|NULL|, the result is never a |REF|
  node, and if it is a |CELL| node, the |t_val| field is empty. }

{ |Deref| -- follow |VAR| and |CELL| pointers }
function Deref(t: term; e: frame): term;
begin
  if t = 0 then begin writeln; writeln('Panic: ', 'Deref'); halt end;
  if (mem[t] div 256 = 5) and (e <> 0) then
    t := (e+7+(mem[t+2]-1)*3);
  while (mem[t] div 256 = 4) and (mem[t+2] <> 0) do
    t := mem[t+2];
  Deref := t
end;

{ This is a good place to put the forward declarations of a few
  procedures and functions. }
procedure PrintTerm(t: term; e: frame; prio: integer); forward;
function ParseTerm: term; forward;
function DoBuiltin(action: integer): boolean; forward;
procedure Collect; forward;
function Key(t: term; e: frame): integer; forward;

{ In the actual definition of a procedure or function that
  has been declared forward, we repeat the parameter list
  in a call to the macro |\it f w d|.  Standard Pascal requires this
  to be replaced by the empty string, but some implementations
  allow the parameter list to be repeated and check that the
  two lists agree. }


{S Symbol table }

{ The names of relations, functions, constants and variables are
  held in a hash table.  It is organized as a `closed' hash table
  with sequential search: this is simple but leaves much room for
  improvement. The symbol table is not allowed to become more full
  than |HASHFACTOR| per cent, since nearly full hash tables of this kind
  perform rather badly.

  Each symbol has an |s_action| code that has a different non-zero
  value for each built-in relation, and is zero for everything
  else. User-defined relations have a chain of clauses that starts
  at the |s_proc| field and is linked together by the |c_next| fields
  of the clauses. }

type symbol = 1..MAXSYMBOLS;  { index in |symtab| }

var
  nsymbols: 0..MAXSYMBOLS;  { number of symbols }
  symtab: array [1..MAXSYMBOLS] of record
      name: integer;  { print name: index in |charbuf| }
      arity: integer;  { number of arguments or -1 }
      action: integer;  { code if built-in, 0 otherwise }
      proc: clause  { clause chain }
    end;
  cons, eqsym, cutsym, nilsym, notsym: symbol;

{ We define selector macros for symbols, just as for terms }





{ |Lookup| -- convert string to internal symbol }
function Lookup(var name: tempstring): symbol;
  label 1;
  var h, i: integer; p: symbol;
begin
  { Compute the hash function in |h| }
  h := 0; i := 1;
  while name[i] <> chr(0) do
    begin h := (5 * h + ord(name[i])) mod MAXSYMBOLS; i := i+1 end;

  { Search the hash table }
  p := h+1;
  while symtab[p].name <> -1 do begin
    if StringEqual(name, symtab[p].name) then goto 1;
    p := p-1;
    if p = 0 then p := MAXSYMBOLS
  end;

  { Not found: enter a new symbol }
  { Be careful to avoid overflow on 16 bit machines: }
  if nsymbols >= (MAXSYMBOLS div 10) * (HASHFACTOR div 10) then
    begin writeln; writeln('Panic: ', 'out of symbol space'); halt end;
  nsymbols := nsymbols+1;
  symtab[p].name := SaveString(name);
  symtab[p].arity := -1;
  symtab[p].action := 0; symtab[p].proc := 0;

1:
  Lookup := p
end;

type keyword = array [1..8] of char;

{ |Enter| -- define a built-in symbol }
function Enter(name: keyword; arity: integer; action: integer): symbol;
  var s: symbol; i: integer; temp: tempstring;
begin
  i := 1;
  while name[i] <> ' ' do
    begin temp[i] := name[i]; i := i+1 end;
  temp[i] := chr(0); s := Lookup(temp);
  symtab[s].arity := arity; symtab[s].action := action;
  Enter := s
end;

{ Codes for built-in relations }
  { $!/0$ }
  { |call/1| }
  { |plus/3| }
  { |times/3| }
  { |integer/1| }
  { |char/1| }
  { |not/1| }
  { |=/2| }
  { |false/0| }
  { |print/1| }
  { |nl/0| }

{ |InitSymbols| -- initialize and define standard symbols }
procedure InitSymbols;
  var i: integer; dummy: symbol;
begin
  nsymbols := 0;
  for i := 1 to MAXSYMBOLS do symtab[i].name := -1;
  cons   := Enter(':       ', 2, 0);
  cutsym := Enter('!       ', 0, 1);
  eqsym  := Enter('=       ', 2, 8);
  nilsym := Enter('nil     ', 0, 0);
  notsym := Enter('not     ', 1, 7);
  dummy  := Enter('call    ', 1, 2);
  dummy  := Enter('plus    ', 3, 3);
  dummy  := Enter('times   ', 3, 4);
  dummy  := Enter('integer ', 1, 5);
  dummy  := Enter('char    ', 1, 6);
  dummy  := Enter('false   ', 0, 9);
  dummy  := Enter('print   ', 1, 10);
  dummy  := Enter('nl      ', 0, 11)
end;

{ |AddClause| -- insert a clause at the end of its chain }
procedure AddClause(c: clause);
  var s: symbol; p: clause;
begin
  s := mem[mem[c+3]+2];
  if symtab[s].action <> 0 then begin
    begin writeln; write('Error: ', 'can''t add clauses to built-in relation '); run := false end;
    WriteString(symtab[s].name)
  end
  else if symtab[s].proc = 0 then
    symtab[s].proc := c
  else begin
    p := symtab[s].proc;
    while mem[p+2] <> 0 do p := mem[p+2];
    mem[p+2] := c
  end
end;

{S Building terms on the heap }

{ Next, some convenient routines that construct various kinds of
  term in the heap area: they are used by the parsing routines to
  construct the internal representation of the input terms they
  read.  The routine |MakeRef| that is supposed to construct a |REF|
  node in fact returns a pointer to one from a fixed collection.
  This saves space, since all clauses can share the same small
  number of |REF| nodes. }

type argbuf = array [1..MAXARITY] of term;

{ |MakeCompound| -- construct a compound term on the heap }
function MakeCompound(fun: symbol; var arg: argbuf): term;
  var p: term; i, n: integer;
begin
  n := symtab[fun].arity;
  p := HeapAlloc(3+n);
  mem[p] := 256 * 1 + 3+n;
  mem[p+2] := fun;
  for i := 1 to n do mem[p+i+2] := arg[i];
  MakeCompound := p
end;

{ |MakeNode| -- construct a compound term with up to 2 arguments }
function MakeNode(fun: symbol; a1, a2: term): term;
  var arg: argbuf;
begin
  arg[1] := a1; arg[2] := a2;
  MakeNode := MakeCompound(fun, arg)
end;

var refnode: array [1..MAXARITY] of term;

{ |MakeRef| -- return a reference cell prepared earlier }
function MakeRef(offset: integer): term;
begin
  MakeRef := refnode[offset]
end;

{ |MakeInt| -- construct an integer node on the heap }
function MakeInt(i: integer): term;
  var p: term;
begin
  p := HeapAlloc(3);
  mem[p] := 256 * 2 + 3;
  mem[p+2] := i; MakeInt := p
end;

{ |MakeChar| -- construct a character node on the heap }
function MakeChar(c: char): term;
  var p: term;
begin
  p := HeapAlloc(3);
  mem[p] := 256 * 3 + 3;
  mem[p+2] := ord(c); MakeChar := p
end;

{ |MakeString| -- construct a string as a Prolog list of chars }
function MakeString(var s: tempstring): term;
  var p: term; i: integer;
begin
  i := StringLength(s);
  p := MakeNode(nilsym, 0, 0);
  while i > 0 do
    begin p := MakeNode(cons, MakeChar(s[i]), p); i := i-1 end;
  MakeString := p
end;

{ |MakeClause| -- construct a clause on the heap }
function MakeClause(nvars: integer; head: term;
		    var body: argbuf; nbody: integer): clause;
  var p: clause; i: integer;
begin
  p := HeapAlloc(4 + nbody + 1);
  mem[p] := nvars; mem[p+2] := 0; mem[p+3] := head;
  for i := 1 to nbody do mem[(p+4)+i-1] := body[i];
  mem[(p+4)+nbody+1-1] := 0;
  if head = 0 then mem[p+1] := 0
  else mem[p+1] := Key(head, 0);
  MakeClause := p
end;

{S Printing terms }

{ These routines print terms on the user's terminal.  The main
  routine is |PrintTerm|, which prints a term by recursively
  traversing it.  Unbound cells are printed in the form |'L123'|
  (for local cells) or |'G234'| (for global cells): the number is
  computed from the address of the cell.  If the frame is
  |NULL|, reference nodes are printed in the form |'@3'|. }

{ operator priorities }
  { isolated term }
  { function arguments }
  { equals sign }
  { colon }

{ |IsString| -- check if a list represents a string }
function IsString(t: term; e: frame): boolean;
  label 3;
  const limit = 128;
  var i: integer;
begin
  i := 0; t := Deref(t, e);
  while i < limit do begin
    if (mem[t] div 256 <> 1) or (mem[t+2] <> cons) then
      goto 3
    else if mem[Deref(mem[t+1+2], e)] div 256 <> 3 then
      goto 3
    else
      begin i := i+1; t := Deref(mem[t+2+2], e) end
  end;
3:
  IsString := (mem[t] div 256 = 1) and (mem[t+2] = nilsym)
end;

{ |ShowString| -- print a list as a string }
procedure ShowString(t: term; e: frame);
begin
  t := Deref(t, e);
  write('"');
  while mem[t+2] <> nilsym do begin
    write(chr(mem[Deref(mem[t+1+2], e)+2]));
    t := Deref(mem[t+2+2], e)
  end;
  write('"')
end;

{ |PrintCompound| -- print a compound term }
procedure PrintCompound(t: term; e: frame; prio: integer);
  var f: symbol; i: integer;
begin
  f := mem[t+2];
  if f = cons then begin
    { |t| is a list: try printing as a string, or use infix : }
    if IsString(t, e) then
      ShowString(t, e)
    else begin
      if prio < 1 then write('(');
      PrintTerm(mem[t+1+2], e, 1-1);
      write(':');
      PrintTerm(mem[t+2+2], e, 1);
      if prio < 1 then write(')')
    end
  end
  else if f = eqsym then begin
    { |t| is an equation: use infix = }
    if prio < 2 then write('(');
    PrintTerm(mem[t+1+2], e, 2-1);
    write(' = ');
    PrintTerm(mem[t+2+2], e, 2-1);
    if prio < 2 then write(')')
  end
  else if f = notsym then begin
    { |t| is a literal 'not P' }
    write('not ');
    PrintTerm(mem[t+1+2], e, 2)
  end
  else begin
    { use ordinary notation }
    WriteString(symtab[f].name);
    if symtab[f].arity > 0 then begin
      write('(');
      PrintTerm(mem[t+1+2], e, 2);
      for i := 2 to symtab[f].arity do begin
        write(', ');
        PrintTerm(mem[t+i+2], e, 2)
      end;
      write(')')
    end
  end
end;

{ |PrintTerm| -- print a term }
procedure PrintTerm ;
begin
  t := Deref(t, e);
  if t = 0 then
    write('*null-term*')
  else begin
    case mem[t] div 256 of
    1:
      PrintCompound(t, e, prio);
    2:
      write(mem[t+2]:1);
    3:
      write('''', chr(mem[t+2]), '''');
    4:
      if (t >= gsp) then
        write('G', (MEMSIZE - t) div 3:1)
      else
        write('L', (t - hp) div 3:1);
    5:
      write('@', mem[t+2])
    else
      write('*unknown-term(tag=', mem[t] div 256:1, ')*')
    end
  end
end;

{ |PrintClause| -- print a clause }
procedure PrintClause(c: clause);
  var i: integer;
begin
  if c = 0 then
    writeln('*null-clause*')
  else begin
    if mem[c+3] <> 0 then begin
      PrintTerm(mem[c+3], 0, 2);
      write(' ')
    end;
    write(':- ');
    if mem[(c+4)+1-1] <> 0 then begin
      PrintTerm(mem[(c+4)+1-1], 0, 2);
      i := 2;
      while mem[(c+4)+i-1] <> 0 do begin
	write(', ');
	PrintTerm(mem[(c+4)+i-1], 0, 2);
	i := i+1
      end
    end;
    writeln('.')
  end
end;

{S Scanner }

{ The |Scan| procedure that reads the next token of a
  clause or goal from the input, together with some procedures that
  implement a crude form of recovery from syntax errors.

  |Scan| puts an integer code into the global variable |token|; if
  the token is an identifier, a number, or a string, there is another
  global variable that contains its actual value.

  The recovery mechanism skips input text until it finds a full stop
  or (if the input was from the terminal) the end of a line.  It
  then sets |token| to |DOT|, the code for a full stop.  The parser
  routines are designed so that they will never read past a full
  stop, and final recovery from the error is achieved when control
  reaches |ReadClause| again. }

var
  token: integer;  { last token from input }
  tokval: symbol;  { if |token = IDENT|, the identifier}
  tokival: integer;  { if |token = NUMBER|, the number }
  toksval: tempstring;  { if |token = STRCON|, the string }
  errflag: boolean;  { whether recovering from an error }
  errcount: integer;  { number of errors found so far }

{ Possible values for |token|: }
  { identifier: see |tokval| }
  { variable: see |tokval| }
  { number: see |tokival| }
  { char constant: see |tokival| }
  { string constant: see |toksval| }
  { |':-'| }
  { |'('| }
  { |')'| }
  { |','| }
  { |'.'| }
  { |':'| }
  { |'='| }
  { |'not'| }
  { end of file }

{ |syntax_error| -- report a syntax error }


{ |ShowError| -- report error location }
procedure ShowError;
begin
  errflag := true; errcount := errcount+1;
  if not interacting then begin
    write('"'); WriteString(filename);
    write('", line ', lineno:1, ' ')
  end;
  write('Syntax error - ')
end;

{ |Recover| -- discard rest of input clause }
procedure Recover;
  var ch: char;
begin
  if not interacting and (errcount >= 20) then
    begin writeln('Too many errors: I''m giving up'); halt end;
  if token <> 10 then begin
    repeat
      ch := GetChar
    until (ch = '.') or (ch = chr(127))
	or (interacting and (ch = chr(10)));
    token := 10
  end
end;





{ |Scan| -- read one symbol from |infile| into |token|. }
procedure Scan;
  var ch, ch2: char; i: integer;
begin
  ch := GetChar; token := 0;
  while token = 0 do begin
    { Loop after white-space or comment }
    if ch = chr(127) then
      token := 14
    else if (ch = ' ') or (ch = chr(9)) or (ch = chr(10)) then
      ch := GetChar
    else if ((((ch >= 'A') and (ch <= 'Z')) or (ch = '_')) 
  or ((ch >= 'a') and (ch <= 'z'))) then begin
      if (((ch >= 'A') and (ch <= 'Z')) or (ch = '_')) then token := 2
      else token := 1;
      i := 1;
      while ((((ch >= 'A') and (ch <= 'Z')) or (ch = '_')) 
  or ((ch >= 'a') and (ch <= 'z'))) or ((ch >= '0') and (ch <= '9')) do begin
        if i > MAXSTRING then
          begin writeln; writeln('Panic: ', 'identifier too long'); halt end;
        toksval[i] := ch; ch := GetChar; i := i+1
      end;
      PushBack(ch);
      toksval[i] := chr(0); tokval := Lookup(toksval);
      if tokval = notsym then token := 13
    end
    else if ((ch >= '0') and (ch <= '9')) then begin
      token := 3; tokival := 0;
      while ((ch >= '0') and (ch <= '9')) do begin
        tokival := 10 * tokival + (ord(ch) - ord('0'));
        ch := GetChar
      end;
      PushBack(ch)
    end
    else begin
      case ch of
      '(': token := 7;
      ')': token := 8;
      ',': token := 9;
      '.': token := 10;
      '=': token := 12;
      '!': begin token := 1; tokval := cutsym end;
      '/':
	begin
	  ch := GetChar;
	  if ch <> '*' then
	    begin if not errflag then
    begin ShowError; writeln('bad token "/"'); Recover end end
	  else begin
	    ch2 := ' '; ch := GetChar;
	    while (ch <> chr(127)) and not ((ch2 = '*') and (ch = '/')) do
	      begin ch2 := ch; ch := GetChar end;
	    if ch = chr(127) then
	      begin if not errflag then
    begin ShowError; writeln('end of file in comment'); Recover end end
	    else
	      ch := GetChar
	  end
	end;
      ':':
        begin
	  ch := GetChar;
	  if ch = '-' then
	    token := 6
	  else
	    begin PushBack(ch); token := 11 end
        end;
      '''':
	begin
	  token := 4; tokival := ord(GetChar); ch := GetChar;
	  if ch <> '''' then
            begin if not errflag then
    begin ShowError; writeln('missing quote'); Recover end end
	end;
      '"':
	begin
	  token := 5; i := 1; ch := GetChar;
	  while (ch <> '"') and (ch <> chr(10)) do
	    begin toksval[i] := ch; ch := GetChar; i := i+1 end;
	  toksval[i] := chr(0);
	  if ch = chr(10) then begin
	    begin if not errflag then
    begin ShowError; writeln('unterminated string'); Recover end end;
	    PushBack(ch)
	  end
	end
      else
	begin if not errflag then
    begin ShowError; writeln('illegal character "', ch, '"'); Recover end end
      end
    end
  end
end;

{ |PrintToken| -- print a token as a string }
procedure PrintToken(t: integer);
begin
  case t of
  1:
    begin write('identifier '); WriteString(symtab[tokval].name); end;
  2:
    begin write('variable '); WriteString(symtab[tokval].name); end;
  3: write('number');
  4:  write('char constant');
  6:  write('":-"');
  7:   write('"("');
  8:   write('")"');
  9:  write('","');
  10:    write('"."');
  11:  write('":"');
  12:  write('"="');
  5: write('string constant')
  else
    write('unknown token')
  end
end;

{S Variable names }

{ As the parser reads an input clause, the routines here maintain a
  table of variable names and the corresponding run-time offsets in
  a frame for the clause: for each |i|, the name of the variable at
  offset |i| is |vartable[i]|.  Each clause contains only a few
  variables, so linear search is good enough.

  If the input clause turns out to be a goal, the table is saved and
  used again to display the answer when execution succeeds. }

var
  nvars: 0..MAXARITY;  { no. of variables so far }
  vartable: array [1..MAXARITY] of symbol;  { names of the variables }

{ |VarRep| -- look up a variable name }
function VarRep(name: symbol): term;
  var i: integer;
begin
  if nvars = MAXARITY then begin writeln; writeln('Panic: ', 'too many variables'); halt end;
  i := 1; vartable[nvars+1] := name;  { sentinel }
  while name <> vartable[i] do i := i+1;
  if i = nvars+1 then nvars := nvars+1;
  VarRep := MakeRef(i)
end;

{ |ShowAnswer| -- display answer and get response }
function ShowAnswer(bindings: frame): boolean;
  var i: integer; ch: char;
begin
  if nvars = 0 then ShowAnswer := true
  else begin
    for i := 1 to nvars do begin
      writeln;
      WriteString(symtab[vartable[i]].name); write(' = ');
      PrintTerm((bindings+7+(i-1)*3), 0, 2-1)
    end;
    if not interacting then
      begin writeln; ShowAnswer := false end
    else begin
      write(' ? '); flush;
      if eoln then
	begin readln; ShowAnswer := false end
      else
	begin readln(ch); ShowAnswer := (ch = '.') end
    end
  end
end;

{S Parser }

{ Here are the routines that parse input clauses.  They use the
  method of recursive descent, with each class of phrase recognized
  by a single function that consumes the tokens of the phrase and
  returns its value.  Each of these functions follows the convention
  that the first token of its phrase is in the global |token|
  variable when the function is called, and the first token after
  the phrase is in |token| on return.  The value of the function is
  the internal data structure for the term; this is built directly
  in the heap, with variables replaced by |REF| nodes.  Syntax
  errors are handled by skipping to the next full stop, then trying
  again to find a clause. }

{ |Eat| -- check for an expected token and discard it }
procedure Eat(expected: integer);
begin
  if token = expected then
    begin if token <> 10 then Scan end
  else if not errflag then begin
    ShowError;
    write('expected '); PrintToken(expected);
    write(', found '); PrintToken(token); writeln;
    Recover
  end
end;

{ |ParseCompound| -- parse a compound term }
function ParseCompound: term;
  var fun: symbol; arg: argbuf; n: integer;
begin
  fun := tokval; n := 0; Eat(1);
  if token = 7 then begin
    Eat(7); n := 1; arg[1] := ParseTerm;
    while token = 9 do
      begin Eat(9); n := n+1; arg[n] := ParseTerm end;
    Eat(8)
  end;
  if symtab[fun].arity = -1 then
    symtab[fun].arity := n
  else if symtab[fun].arity <> n then
    begin if not errflag then
    begin ShowError; writeln('wrong number of args'); Recover end end;
  ParseCompound := MakeCompound(fun, arg)
end;

{ |ParsePrimary| -- parse a primary }
function ParsePrimary: term;
  var t: term;
begin
  if token = 1 then t := ParseCompound
  else if token = 2 then
    begin t := VarRep(tokval); Eat(2) end
  else if token = 3 then
    begin t := MakeInt(tokival); Eat(3) end
  else if token = 4 then
    begin t := MakeChar(chr(tokival)); Eat(4) end
  else if token = 5 then
    begin t := MakeString(toksval); Eat(5) end
  else if token = 7 then
    begin Eat(7); t := ParseTerm; Eat(8) end
  else begin
    begin if not errflag then
    begin ShowError; writeln('expected a term'); Recover end end; t := 0
  end;
  ParsePrimary := t
end;

{ |ParseFactor| -- parse a factor }
function ParseFactor: term;
  var t: term;
begin
  t := ParsePrimary;
  if token <> 11 then
    ParseFactor := t
  else begin
    Eat(11);
    ParseFactor := MakeNode(cons, t, ParseFactor)
  end
end;

{ |ParseTerm| -- parse a term }
function ParseTerm ;
  var t: term;
 begin
  t := ParseFactor;
  if token <> 12 then
    ParseTerm := t
  else begin
    Eat(12);
    ParseTerm := MakeNode(eqsym, t, ParseFactor)
  end
end;

{ |CheckAtom| -- check that a literal is a compound term }
procedure CheckAtom(a: term);
begin
  if mem[a] div 256 <> 1 then
    begin if not errflag then
    begin ShowError; writeln('literal must be a compound term'); Recover end end
end;

{ |ParseClause| -- parse a clause }
function ParseClause(isgoal: boolean): clause;
  label 3;
  var head, t: term; 
    body: argbuf; 
    n: integer;
    minus: boolean;
begin
  if isgoal then 
    head := 0
  else begin
    head := ParseTerm; 
    CheckAtom(head);
    Eat(6)
  end;

  n := 0;
  if token <> 10 then begin
    while true do begin
      n := n+1; minus := false;
      if token = 13 then
	begin Eat(13); minus := true end;
      t := ParseTerm; CheckAtom(t);
      if minus then body[n] := MakeNode(notsym, t, 0)
      else body[n] := t;
      if token <> 9 then goto 3;
      Eat(9)
    end
  end;
3:
  Eat(10);

  if errflag then ParseClause := 0
  else ParseClause := MakeClause(nvars, head, body, n)
end;

{ |ReadClause| -- read a clause from |infile| }
function ReadClause: clause;
  var c: clause;
begin
  repeat
    hp := hmark; nvars := 0; errflag := false;
    if interacting then
      begin writeln; write('# :- '); flush end;
    Scan;
    if token = 14 then c := 0
    else c := ParseClause(interacting)
  until (not errflag) or (token = 14);
  ReadClause := c
end;

{S Trail }

{ The trail stack records assignments made to variables, so that
  they can be undone on backtracking.  It is a linked list of nodes
  with a |t_kind| of |UNDO| allocated from the global stack.  The
  variables for which bindings are actually kept in the trail are the
  `critical' ones that will not be destroyed on backtracking. }

type trail = pointer;
{ Nodes on the trail share the |t_tag| and |t_shift| fields of
  other nodes on the global stack, plus: }
  { variable to reset }
  { next trail entry }


var trhead: trail;  { start of the trail }

{ |critical| -- test if a variable will survive backtracking }


{ |Save| -- add a variable to the trail if it is critical }
procedure Save(v: term);
  var p: trail;
begin
  if ((v < choice) or (v >= mem[choice+4])) then begin
    p := GloAlloc(6, 4);
    mem[p+2] := v; mem[p+3] := trhead; trhead := p
  end
end;

{ |Restore| -- undo bindings back to previous state }
procedure Restore;
  var v: term;
begin
  while (trhead <> mem[choice+5]) do begin
    v := mem[trhead+2];
    if v <> 0 then mem[v+2] := 0;
    trhead := mem[trhead+3]
  end
end;

{ |Commit| -- blank out trail entries not needed after cut }
procedure Commit;
  var p: trail;
begin
  p := trhead;
  if choice = 0 then begin writeln; write('Error: ', 'Commit'); run := false end;
  while (p <> 0) and (p < mem[choice+4]) do begin
    if (mem[p+2] <> 0) and not ((mem[p+2] < choice) or (mem[p+2] >= mem[choice+4])) then
      mem[p+2] := 0;
    p := mem[p+3]
  end
end;

{S Unification }

{ The unification algorithm is the naive one that is traditional in
  Prolog implementations.  Tradition is also followed in omitting
  the `occur check'.

  Nodes of type |CELL| may only point to terms that are independent
  of any frame: i.e., they may not point to terms in the heap
  that may contain |REF| nodes.  So there is a function |GloCopy|
  that copies out enough of a term onto the global stack so that any
  cell can point to it.  No copy is needed if the term is already on
  the global stack, or if it is a simple term that cannot contain
  any |REF|'s. }

{ |GloCopy| -- copy a term onto the global stack }
function GloCopy(t: term; e: frame): term;
  var tt: term; i, n: integer;
begin
  t := Deref(t, e);
  if (t >= gsp) then
    GloCopy := t
  else begin
    case mem[t] div 256 of
    1:
      begin
	n := symtab[mem[t+2]].arity;
	if (t <= hp) and (n = 0) then GloCopy := t
	else begin
	  tt := GloAlloc(1, 3+n);
	  mem[tt+2] := mem[t+2];
	  for i := 1 to n do
	    mem[tt+i+2] := GloCopy(mem[t+i+2], e);
	  GloCopy := tt
	end
      end;
    4:
      begin
        tt := GloAlloc(4, 3);
        mem[tt+2] := 0;
	Save(t); mem[t+2] := tt;
        GloCopy := tt
      end;
    2, 3:
      GloCopy := t
    else
      begin writeln; writeln('Panic: ', 'bad tag ', mem[t] div 256:1, ' in ', 'GloCopy'); halt end
    end
  end
end;

{ When two variables are made to `share', there is a choice of which
  variable is made to point to the other.  The code takes care to
  obey some rules about what may point to what: (1) Nothing on the
  global stack may point to anything on the local stack; (2) Nothing
  on the local stack may point to anything nearer the top of the
  local stack.  Both these rules are necessary, since the top part
  of the local stack may be reclaimed without warning.  There is
  another rule that makes for better performance: (3) Avoid pointers
  from items nearer the bottom of the global stack to items nearer
  the top.

  The tricky |lifetime| macro implements these rules by
  computing a numerical measure of the lifetime of an object,
  defined so that anything on the local stack is shorter-lived than
  anything on the global stack, and within each stack items near the
  top are shorter-lived than items near the bottom. }

{ |lifetime| -- measure of potential lifetime }


{ |Share| -- bind two variables together }
procedure Share(v1, v2: term);
begin
  if (v1 * (2 * ord((v1 >= gsp)) - 1)) <= (v2 * (2 * ord((v2 >= gsp)) - 1)) then
    begin Save(v1); mem[v1+2] := v2 end
  else
    begin Save(v2); mem[v2+2] := v1 end
end;

{ |Unify| -- find and apply unifier for two terms }
function Unify(t1: term; e1: frame; t2: term; e2: frame): boolean;
  var i: integer; match: boolean;
begin
  t1 := Deref(t1, e1); t2 := Deref(t2, e2);
  if t1 = t2 then  { Includes unifying a var with itself }
    Unify := true
  else if (mem[t1] div 256 = 4) and (mem[t2] div 256 = 4) then
    begin Share(t1, t2); Unify := true end
  else if mem[t1] div 256 = 4 then
    begin Save(t1); mem[t1+2] := GloCopy(t2, e2); Unify := true end
  else if mem[t2] div 256 = 4 then
    begin Save(t2); mem[t2+2] := GloCopy(t1, e1); Unify := true end
  else if mem[t1] div 256 <> mem[t2] div 256 then
    Unify := false
  else begin
    case mem[t1] div 256 of
    1:
      if (mem[t1+2] <> mem[t2+2]) then
        Unify := false
      else begin
        i := 1; match := true;
        while match and (i <= symtab[mem[t1+2]].arity) do begin
          match := Unify(mem[t1+i+2], e1, mem[t2+i+2], e2);
          i := i+1
        end;
        Unify := match
      end;
    2:
      Unify := (mem[t1+2] = mem[t2+2]);
    3:
      Unify := (mem[t1+2] = mem[t2+2])
    else
      begin writeln; writeln('Panic: ', 'bad tag ', mem[t1] div 256:1, ' in ', 'Unify'); halt end
    end
  end
end;

{ |Key| -- unification key of a term }
function Key ;
  var t0: term;
begin
  { The argument |t| must be a direct pointer to a compound term.
    The value returned is |key(t)|: if |t1| and |t2| are unifiable,
    then |key(t1) = 0| or |key(t2) = 0| or |key(t1) = key(t2)|. }

  if t = 0 then begin writeln; writeln('Panic: ', 'Key'); halt end;
  if mem[t] div 256 <> 1 then begin writeln; writeln('Panic: ', 'bad tag ', mem[t] div 256:1, ' in ', 'Key1'); halt end;

  if symtab[mem[t+2]].arity = 0 then
    Key := 0
  else begin
    t0 := Deref(mem[t+1+2], e);
    case mem[t0] div 256 of
      1:      Key := mem[t0+2];
      2:       Key := mem[t0+2] + 1;
      3:    Key := mem[t0+2] + 1;
      5, 4: Key := 0
    else
      begin writeln; writeln('Panic: ', 'bad tag ', mem[t0] div 256:1, ' in ', 'Key2'); halt end
    end
  end
end;

{ |Search| -- find the first clause that might match }
function Search(t: term; e: frame; p: clause): clause;
  var k: integer;
begin
  k := Key(t, e);
  if k <> 0 then
    while (p <> 0) and (mem[p+1] <> 0) and (mem[p+1] <> k) do
      p := mem[p+2];
  Search := p
end;

{S Interpreter }

{ The main control of the interpreter uses a depth-first search
  procedure with an explicit stack of activation records.  It
  includes the tail-recursion optimization and an indexing scheme
  that uses the hash codes computed by |Key|. }

var ok: boolean;  { whether execution succeeded }



{ |PushFrame| -- create a new local stack frame }
procedure PushFrame(nvars: integer; retry: clause);
  var f: frame; i: integer;
begin
  f := LocAlloc((7 + (nvars)*3));
  mem[f] := current; mem[f+1] := goalframe;
  mem[f+2] := retry; mem[f+3] := choice;
  mem[f+4] := gsp; mem[f+5] := trhead;
  mem[f+6] := nvars;
  for i := 1 to nvars do begin
    mem[(f+7+(i-1)*3)] := 256 * 4 + 3;
    mem[(f+7+(i-1)*3)+2] := 0
  end;
  goalframe := f;
  if retry <> 0 then choice := goalframe
end;

{ Tail recursion can be used only under rather stringent conditions:
  the goal literal must be the last one in the body of the calling
  clause, both the calling clause and the called clause must be
  determinate, and the calling clause must not be the original goal
  (lest the answer variables be lost).  The macro |tro_test(p)|
  checks that these conditions are satisfied, where |p| is the
  untried part of the procedure for the current goal literal. }

{ |tro_test| -- test if a resolution step can use TRO }


{ If the |tro_test| macro returns true, then it is safe to discard
  the calling frame in a resolution step before solving the subgoals
  in the newly-created frame.  |TroStep| implements this manoeuvre:
  read it after you understand the normal case covered by |Step|.

  Because the calling frame is to be discarded, it is important that
  no pointers from the new frame to the calling frame are created
  during unification.  |TroStep| uses the trick of swapping the two
  frames so that |Unify| will make pointers go the right way.  The
  idea is simple, but the details are made complicated by the need
  to adjust internal pointers in the relocated frame. }

{ |TroStep| -- perform a resolution step with tail-recursion }
procedure TroStep;
  var temp: frame; oldsize, newsize, i: integer;
begin
  if dflag then writeln('(TRO)');

  oldsize := (7 + (mem[goalframe+6])*3); { size of old frame }
  newsize := (7 + (mem[proc])*3); { size of new frame }
  temp := LocAlloc(newsize);
  temp := goalframe + newsize; { copy old frame here }

  { Copy the old frame: in reverse order in case of overlap }
  for i := oldsize-1 downto 0 do mem[temp+i] := mem[goalframe+i];

  { Adjust internal pointers in the copy }
  for i := 1 to mem[goalframe+6] do begin
    if (mem[(temp+7+(i-1)*3)] div 256 = 4)
        and (mem[(temp+7+(i-1)*3)+2] <> 0)
        and (goalframe <= mem[(temp+7+(i-1)*3)+2])
        and (mem[(temp+7+(i-1)*3)+2] < goalframe + oldsize) then
      mem[(temp+7+(i-1)*3)+2] := mem[(temp+7+(i-1)*3)+2] + newsize
  end;

  { Overwrite the old frame with the new one }
  mem[goalframe+6] := mem[proc];
  for i := 1 to mem[goalframe+6] do begin
    mem[(goalframe+7+(i-1)*3)] := 256 * 4 + 3;
    mem[(goalframe+7+(i-1)*3)+2] := 0
  end;

  { Perform the resolution step }
  ok := Unify(call, temp, mem[proc+3], goalframe);
  current := (proc+4);
  lsp := temp-1
end;

{ The |Step| procedure carries out a single resolution step.
  Built-in relations are treated as a special case; so are
  resolution steps that can use the tail-recursion optimization.
  Otherwise, we allocate a frame for the first clause for the
  current goal literal, unify the clause head with the literal, and adopt
  the clause body as the new goal.  The step can fail (and |Step|
  returns |false|) if there are no clauses to try, or if the first
  clause fails to match. }

{ |Step| -- perform a resolution step }
procedure Step;
  var retry: clause;
begin
  if symtab[mem[call+2]].action <> 0 then
    ok := DoBuiltin(symtab[mem[call+2]].action)
  else if proc = 0 then
    ok := false
  else begin
    retry := Search(call, goalframe, mem[proc+2]);
    if (mem[(current)+1] = 0) and (choice < goalframe)
    and (retry = 0) and (goalframe <> base) then
      TroStep
    else begin
      PushFrame(mem[proc], retry);
      ok := Unify(call, mem[goalframe+1], mem[proc+3], goalframe);
      current := (proc+4);
    end
  end
end;

{ The |Unwind| procedure returns from completed clauses until it
  finds one where there is still work to do, or it finds that the
  original goal is completed.  At this point, completed frames are
  discarded if they cannot take part in future backtracking. }

{ |Unwind| -- return from completed clauses }
procedure Unwind;
begin
  while (mem[current] = 0) and (goalframe <> base) do begin
    if dflag then begin write('Exit', ': ');
    PrintTerm(mem[mem[goalframe]], mem[goalframe+1], 2); writeln end;
    current := (mem[goalframe])+1;
    if goalframe > choice then lsp := goalframe-1;
    goalframe := mem[goalframe+1]
  end
end;

{ The |Backtrack| procedure undoes all the work that has been done
  since the last non-deterministic choice (indicated by the |choice|
  register).  The trail shows what assignments must be undone, and
  the stacks are returned to the state they were in when the choice
  was made.  The |proc| register is set from the |f_retry| field of
  the |choice| frame: this is the list of clauses for that goal that
  remain to be tried }

{ |Backtrack| -- roll back to the last choice-point }
procedure Backtrack;
begin
  Restore;
  current := mem[choice]; goalframe := mem[choice+1];
  call := Deref(mem[current], goalframe);
  proc := mem[choice+2]; gsp := mem[choice+4];
  lsp := choice-1; choice := mem[choice+3];
  if dflag then begin write('Redo', ': ');
    PrintTerm(call, goalframe, 2); writeln end;
end;

{ |Resume| is called with |ok = true| when the interpreter starts to
  execute a goal; it either returns with |ok = true| when the goal
  succeeds, or returns with |ok = false| when it has completely
  failed.  After |Resume| has returned |true|, it can be called
  again with |ok = false| to find another solution; in this case,
  the first action is to backtrack to the most recent choice-point. }

{ |Resume| -- continue execution }
procedure Resume;
  label 2;
begin
  while run do begin
    if ok then begin
      if mem[current] = 0 then goto 2;
      call := Deref(mem[current], goalframe);
      if dflag then begin write('Call', ': ');
    PrintTerm(call, goalframe, 2); writeln end;
      if (symtab[mem[call+2]].proc = 0)
	  and (symtab[mem[call+2]].action = 0) then begin
	begin writeln; write('Error: ', 'call to undefined relation '); run := false end;
	WriteString(symtab[mem[call+2]].name);
	goto 2
      end;
      proc := Search(call, goalframe, symtab[mem[call+2]].proc)
    end
    else begin
      if choice <= base then goto 2;
      Backtrack
    end;
    Step;
    if ok then Unwind;
    if gsp - lsp <= GCLOW then Collect
  end;
2:
end;

{ |Execute| -- solve a goal by SLD-resolution }
procedure Execute(g: clause);
  label 2;
begin
  lsp := hp; gsp := MEMSIZE+1;
  current := 0; goalframe := 0; choice := 0; trhead := 0;
  PushFrame(mem[g], 0);
  choice := goalframe; base := goalframe; current := (g+4);
  mem[base+3] := base;
  run := true; ok := true;
  repeat
    Resume;
    if not run then goto 2;
    if not ok then
      begin writeln; write('no'); goto 2 end;
    ok := ShowAnswer(base)
  until ok;
  writeln; write('yes');
2:
end;

{S Built-in relations }

{ Each built-in relation is a parameterless boolean-valued
  function: it finds its arguments from the call in |call|, carries
  out whatever side-effect is desired, and returns |true| exactly if
  the call succeeds.

  Two routines help in defining built-in relations: |GetArgs|
  dereferences the argument of the literal |call| and puts them in
  the global array |av|; and |NewInt| makes a new integer node on 
  the global stack. }

var
  av: argbuf;  { |GetArgs| puts arguments here }
  callbody: pointer;  { dummy clause body used by |call/1| }

{ |GetArgs| -- set up |av| array }
procedure GetArgs;
  var i: integer;
begin
  for i := 1 to symtab[mem[call+2]].arity do
    av[i] := Deref(mem[call+i+2], goalframe)
end;

{ A couple of macros that abbreviate accesses to the |av| array: }



function NewInt(n: integer): term;
  var t: term;
begin
  t := GloAlloc(2, 3);
  mem[t+2] := n;
  NewInt := t
end;

{ |DoCut| -- built-in relation !/0 }
function DoCut: boolean;
begin
  choice := mem[goalframe+3];
  lsp := goalframe + (7 + (mem[goalframe+6])*3) - 1;
  Commit;
  current := (current)+1;
  DoCut := true
end;

{ |DoCall| -- built-in relation |call/1| }
function DoCall: boolean;
begin
  GetArgs;
  if not (mem[av[1]] div 256 = 1) then begin
    begin writeln; write('Error: ', 'bad argument to call/1'); run := false end;
    DoCall := false
  end
  else begin
    PushFrame(1, 0);
    mem[(goalframe+7+(1-1)*3)+2] :=
      GloCopy(av[1], mem[goalframe+1]);
    current := callbody;
    DoCall := true
  end
end;

{ |DoNot| -- built-in relation |not/1| }
function DoNot: boolean;
  var savebase: frame;
begin
  GetArgs;
  if not (mem[av[1]] div 256 = 1) then begin
    begin writeln; write('Error: ', 'bad argument to call/1'); run := false end;
    DoNot := false
  end
  else begin
    PushFrame(1, 0);
    savebase := base; base := goalframe; choice := goalframe;
    mem[(goalframe+7+(1-1)*3)+2] :=
      GloCopy(av[1], mem[goalframe+1]);
    current := callbody; ok := true;
    Resume;
    choice := mem[base+3]; goalframe := mem[base+1];
    if not ok then begin
      current := (mem[base])+1;
      DoNot := true
    end
    else begin
      Commit;
      DoNot := false
    end;
    lsp := base-1; base := savebase
  end
end;

{ Procedures |DoPlus| and |DoTimes| implement the |plus/3| and
  |times/3| relations: they both involve a case analysis of which
  arguments are known, followed by a call to |Unify| to unify the
  remaining argument with the result.  The |times/3| relation fails
  on divide-by-zero, even in the case |times(X, 0, 0)|, which
  actually has infinitely many solutions. }

{ |DoPlus| -- built-in relation |plus/3| }
function DoPlus: boolean;
  var result: boolean;
begin
  GetArgs;
  result := false;
  if (mem[av[1]] div 256 = 2) and (mem[av[2]] div 256 = 2) then
    result := Unify(av[3], goalframe, NewInt(mem[av[1]+2] + mem[av[2]+2]), 0)
  else if (mem[av[1]] div 256 = 2) and (mem[av[3]] div 256 = 2) then begin
    if mem[av[1]+2] <= mem[av[3]+2] then
      result := Unify(av[2], goalframe, 
		      NewInt(mem[av[3]+2] - mem[av[1]+2]), 0)
  end
  else if (mem[av[2]] div 256 = 2) and (mem[av[3]] div 256 = 2) then begin
    if mem[av[2]+2] <= mem[av[3]+2] then
      result := Unify(av[1], goalframe, NewInt(mem[av[3]+2] - mem[av[2]+2]), 0)
  end
  else
    begin writeln; write('Error: ', 'plus/3 needs at least two integers'); run := false end;
  current := (current)+1;
  DoPlus := result
end;

{ |DoTimes| -- built-in relation |times/3| }
function DoTimes: boolean;
  var result: boolean;
begin
  GetArgs;
  result := false;
  if (mem[av[1]] div 256 = 2) and (mem[av[2]] div 256 = 2) then
    result := Unify(av[3], goalframe, 
		    NewInt(mem[av[1]+2] * mem[av[2]+2]), 0)
  else if (mem[av[1]] div 256 = 2) and (mem[av[3]] div 256 = 2) then begin
    if mem[av[1]+2] <> 0 then
      if mem[av[3]+2] mod mem[av[1]+2] = 0 then
        result := Unify(av[2], goalframe, 
		        NewInt(mem[av[3]+2] div mem[av[1]+2]), 0)
  end
  else if (mem[av[2]] div 256 = 2) and (mem[av[3]] div 256 = 2) then begin
    if mem[av[2]+2] <> 0 then
      if mem[av[3]+2] mod mem[av[2]+2] = 0 then
        result := Unify(av[1], goalframe, 
			NewInt(mem[av[3]+2] div mem[av[2]+2]), 0)
  end
  else
    begin writeln; write('Error: ', 'times/3 needs at least two integers'); run := false end;
  current := (current)+1;
  DoTimes := result
end;

{ |DoEqual| -- built-in relation |=/2| }
function DoEqual: boolean;
begin
  GetArgs;
  current := (current)+1;
  DoEqual := Unify(av[1], goalframe, av[2], goalframe)
end;

{ |DoInteger| -- built-in relation |integer/1| }
function DoInteger: boolean;
begin
  GetArgs;
  current := (current)+1;
  DoInteger := (mem[av[1]] div 256 = 2)
end;

{ |DoChar| -- built-in relation |char/1| }
function DoChar: boolean;
begin
  GetArgs;
  current := (current)+1;
  DoChar := (mem[av[1]] div 256 = 3)
end;

{ |DoPrint| -- built-in relation |print/1| }
function DoPrint: boolean;
begin
  GetArgs;
  PrintTerm(av[1], goalframe, 2);
  current := (current)+1;
  DoPrint := true
end;

{ |DoNl| -- built-in relation |nl/0| }
function DoNl: boolean;
begin
  writeln;
  current := (current)+1;
  DoNl := true
end;  

{ |DoBuiltin| -- switch for built-in relations }
function DoBuiltin ;
begin
  case action of
  1:      DoBuiltin := DoCut;
  2:     DoBuiltin := DoCall;
  3:     DoBuiltin := DoPlus;
  4:    DoBuiltin := DoTimes;
  5:    DoBuiltin := DoInteger;
  6:   DoBuiltin := DoChar;
  7:     DoBuiltin := DoNot;
  8: DoBuiltin := DoEqual;
  9:	    DoBuiltin := false;
  10:    DoBuiltin := DoPrint;
  11:	    DoBuiltin := DoNl
  else
    begin writeln; writeln('Panic: ', 'bad tag ', action:1, ' in ', 'DoBuiltin'); halt end
  end
end;

{S Garbage collection }

{ Finally, here is the garbage collector, which reclaims space in the
  global stack that is no longer accessible.  It must work well with
  the stack-like expansion and contraction of the stack, so it is a
  compacting collector that does not alter the order in memory of
  the accessible nodes.

  The garbage collector operates in four phases: (1) Find and mark
  all accessible storage.  (2) Compute the new positions of the
  marked items after the global stack is compacted.  (3) Adjust all
  pointers to marked items.  (4) Compact the global stack and move
  it to the top of |mem|.  That may seem complicated, and it is; the
  garbage collector must know about all the run-time data
  structures, and is that one piece of the system that cuts across
  every abstraction boundary.

  Because of the relocation, |Collect| should only be called at
  `quiet' times, when the only pointers into the global stack are
  from interpreter registers and the local stack.  An example of a
  `non-quiet' time is in the middle of unification, when many
  recursive copies of the unification procedure are keeping pointers
  to bits of term structure.  To avoid the need to collect garbage
  at such times, the main control of the interpreter calls |Collect|
  before each resolution step if the space left is less than
  |GCLOW|.  If space runs out in the subsequent resolution step,
  execution is abandoned without much grace.  This plan works
  because the amount of space consumed in a resolution step is
  bounded by the maximum size of a program clause; this size is not
  checked, though. }

var shift: integer;  { amount global stack will shift }

{ |Visit| -- recursively mark a term and all its sub-terms }
procedure Visit(t: term);
  label 2;
  var i, n: integer;
begin
  { We reduce the depth of recursion when marking long lists by
    treating the last argument of a function iteratively, making
    recursive calls only for the other arguments. }
  while t <> 0 do begin
    if not (t >= gsp) or (mem[t] mod 256 >= 128) then goto 2;
    mem[t] := mem[t] + 128;
    case mem[t] div 256 of
    1:
      begin
	n := symtab[mem[t+2]].arity;
	if n = 0 then goto 2;
        for i := 1 to n-1 do Visit(mem[t+i+2]);
        t := mem[t+n+2]
      end;
    4:
      t := mem[t+2];
    2, 3:
      goto 2
    else
      begin writeln; writeln('Panic: ', 'bad tag ', mem[t] div 256:1, ' in ', 'Visit'); halt end
    end
  end;
2:
end;

{ |MarkStack| -- mark from each frame on the local stack }
procedure MarkStack;
  var f: frame; i: integer;
begin
  f := hp+1;
  while f <= lsp do begin
    for i := 1 to mem[f+6] do
      if mem[(f+7+(i-1)*3)] div 256 = 4 then
	Visit(mem[(f+7+(i-1)*3)+2]);
    f := f + (7 + (mem[f+6])*3)
  end
end;

{ |CullTrail| -- delete an initial segment of unwanted trail }
procedure CullTrail(var p: trail);
  label 2;
begin
  while p <> 0 do begin
    if mem[p+2] <> 0 then
      if not (mem[p+2] >= gsp) or (mem[mem[p+2]] mod 256 >= 128) then
	goto 2;
    p := mem[p+3]
  end;
2:
end;

{ |MarkTrail| -- remove dead trail nodes, mark the rest. }
procedure MarkTrail;
  var p: trail;
begin
  CullTrail(trhead); p := trhead;
  while p <> 0 do
    begin mem[p] := mem[p] + 128; CullTrail(mem[p+3]); p := mem[p+3] end
end;

{ |Relocate| -- compute shifts }
procedure Relocate;
  var p: pointer; step: integer;
begin
  shift := 0; p := gsp;
  while p <= MEMSIZE do begin
    step := mem[p] mod 128; mem[p+1] := shift;
    if not (mem[p] mod 256 >= 128) then
      shift := shift + step;
    p := p + step
  end
end;

{ |AdjustPointer| -- update a pointer}
procedure AdjustPointer(var p: term);
begin
  if (p <> 0) and (p >= gsp) then begin
    if not (mem[p] mod 256 >= 128) then
      begin writeln; writeln('Panic: ', 'adjusting pointer to unmarked block'); halt end;
    p := p + shift - mem[p+1]
  end
end;

{ |AdjustStack| -- adjust pointers in local stack }
procedure AdjustStack;
  var f: frame; i: integer; q: pointer;
  label 1, 4;
begin
  f := hp+1;
  while f <= lsp do begin
    q := mem[f+4];
    while q <= MEMSIZE do begin
      if (mem[q] mod 256 >= 128) then goto 1;
      q := q + mem[q] mod 128
    end;
  1:
    if q <= MEMSIZE then AdjustPointer(q);
    mem[f+4] := q;

    q := mem[f+5];
    while q <> 0 do begin
      if (mem[q] mod 256 >= 128) then goto 4;
      q := mem[q+3]
    end;
  4:
    AdjustPointer(q);
    mem[f+5] := q;

    for i := 1 to mem[f+6] do
      if mem[(f+7+(i-1)*3)] div 256 = 4 then
	AdjustPointer(mem[(f+7+(i-1)*3)+2]);
    f := f + (7 + (mem[f+6])*3);
  end
end;

{ |AdjustInternal| -- update internal pointers }
procedure AdjustInternal;
  var p, i: integer;
begin
  p := gsp;
  while p <= MEMSIZE do begin
    if (mem[p] mod 256 >= 128) then begin
      case mem[p] div 256 of
      1:
        for i := 1 to symtab[mem[p+2]].arity do
	  AdjustPointer(mem[p+i+2]);
      4:
        AdjustPointer(mem[p+2]);
      6:
	begin
	  AdjustPointer(mem[p+2]);
	  AdjustPointer(mem[p+3])
	end;
      2, 3:
        
      else
	begin writeln; writeln('Panic: ', 'bad tag ', mem[p] div 256:1, ' in ', 'Adjust'); halt end
      end
    end;
    p := p + mem[p] mod 128
  end
end;

{ |Compact| -- compact marked blocks and un-mark }
procedure Compact;
  var p, q, step, i: integer;
begin
  p := gsp; q := gsp;
  while p <= MEMSIZE do begin
    step := mem[p] mod 128;
    if (mem[p] mod 256 >= 128) then begin mem[p] := mem[p] - 128;
      for i := 0 to step-1 do mem[q+i] := mem[p+i];
      q := q + step
    end;
    p := p + step
  end;
  gsp := gsp+shift;
  for i := MEMSIZE downto gsp do mem[i] := mem[i-shift];
end;

{ |Collect| -- collect garbage }
procedure Collect;
begin
  write('[gc'); flush;

  { Phase 1: marking }
  Visit(call); MarkStack; MarkTrail;

  { Phase 2: compute new locations }
  Relocate;

  { Phase 3: adjust pointers }
  AdjustPointer(call); AdjustPointer(trhead);
  AdjustStack; AdjustInternal;

  { Phase 4: compact }
  Compact;

  write(']'); flush;
  if gsp - lsp <= GCHIGH then begin writeln; write('Error: ', 'out of memory space'); run := false end
end;

{S Main program }

{ |Initialize| -- initialize everything }
procedure Initialize;
  var i: integer; p: term;
begin
  dflag := false; errcount := 0;
  pbchar := chr(127); charptr := 0;
  hp := 0; InitSymbols;

  { Set up the |refnode| array }
  for i := 1 to MAXARITY do begin
    p := HeapAlloc(3);
    mem[p] := 256 * 5 + 3;
    mem[p+2] := i; refnode[i] := p
  end;

  { The dummy clause $\it call(\sci p) \IF p$ is used by |call/1|. }
  callbody := HeapAlloc(2);
  mem[callbody] := MakeRef(1);
  mem[(callbody)+1] := 0
end;

{ |ReadFile| -- read and process clauses from an open file }
procedure ReadFile;
  var c: clause;
begin
  lineno := 1;
  repeat
    hmark := hp;
    c := ReadClause;
    if c <> 0 then begin
      if dflag then PrintClause(c);	
      if mem[c+3] <> 0 then
        AddClause(c)
      else begin
        if interacting then
	  begin pbchar := chr(127); readln end;
        Execute(c);
	writeln;
	hp := hmark
      end
    end
  until c = 0
end;

{ |ReadProgram| -- read files listed on command line }
procedure ReadProgram;
  var i0, i: integer;
    arg: tempstring;
begin
  i0 := 1;
  if argc > 1 then begin
    argv(1, arg);
    if (arg[1] = '-') and (arg[2] = 'd')
	and (arg[3] = chr(0)) then begin
      dflag := true;
      i0 := i0+1
    end
  end;
  for i := i0 to argc-1 do begin
    argv(i, arg);
    filename := SaveString(arg);
    if not openin(infile, arg) then begin
      write('Can''t read '); WriteString(filename); writeln;
      halt
    end;
    write('Reading '); WriteString(filename); writeln;
    ReadFile;
    closein(infile);
    if errcount > 0 then halt
  end
end;

begin  { main program }
  writeln('Welcome to picoProlog');
  Initialize;
  interacting := false; ReadProgram;
  interacting := true; lineno := 1; ReadFile;
  writeln;
999:
end.
