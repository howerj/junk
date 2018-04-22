filetype plugin on
filetype indent on

set ignorecase
set smartcase
set hlsearch
set incsearch 
set lazyredraw 

syntax enable 

set encoding=utf8
set ffs=unix,dos,mac

set nobackup
set nowb
set noswapfile
set smarttab

set ai "Auto indent
set si "Smart indent
set nowrap "Wrap lines
set ruler

set backspace=eol,start,indent
set whichwrap+=<,>,h,l

set hid

if has("gui_win32")
	set guifont=Fixedsys:h14:cANSI
	" https://superuser.com/questions/140419/how-to-start-gvim-maximized
	au GUIEnter * simalt ~x
else
	"set lines=999 columns=999
endif

" Set extra options when running in GUI mode
if has("gui_running")
	set guioptions-=T
	set guioptions-=e
	set t_Co=256
	set guitablabel=%M\ %t
endif

set guioptions-=r
set guioptions-=R
set guioptions-=l
set guioptions-=L

set spell spelllang=en

au BufRead,BufNewFile *.md set filetype=markdown
au BufRead,BufNewFile *.fth set colorcolumn=80


