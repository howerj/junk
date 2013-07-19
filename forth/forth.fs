: cpf 8 exit 
: state cpf ! exit
: ; immediate 
	' exit , 
	false state
exit

: r 1 ;
: h 0 ;
: here h @ ;

: [ immediate 1 state ;
: ] 0 state ;

: if immediate ' jmpz , here 0 , ;
: then immediate dup here swap - swap ! ;

: '\n' 10 ;
: '"' 34 ;
: 'space' 32 ;
: ')' 41 ;
: '0' 48 ;

: cr '\n' emit exit

: find-) key ')' = not if tail find-) then ;

: ( immediate find-) ;

: else immediate
  ' jmp ,       
  here           
  0 ,            
  swap           
  dup here swap -
  swap !       
;


: _print
  dup 1 +
  swap @
  dup '"' =
  if
    drop exit
  then
  emit
  tail _print
;

: print _print ;

  ( print the next thing from the instruction stream )
: immprint
  r @ @
  print
  r @ !
;

: find-"
  key dup ,
  '"' =
  if
    exit
  then
  tail find-"
;

: " immediate
  key drop
  ' immprint ,
  find-"
;

: _.( key dup ')' = if drop exit then emit tail _.( ;
: .( key drop _.( ;

.( 
  Welcome to third.
  Ok.
) cr
