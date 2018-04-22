
syntax on

" If using a dark background within the editing area and syntax highlighting
" turn on this option as well
set background=dark

if has("autocmd")
  filetype plugin indent on
endif

au BufRead,BufNewFile *.md  set filetype=markdown
au BufRead,BufNewFile *.4th set syntax=forth
au BufRead,BufNewFile *.fth set syntax=forth

" The following are commented out as they cause vim to behave a lot
" differently from regular Vi. They are highly recommended though.
set showcmd		" Show (partial) command in status line.
set showmatch		" Show matching brackets.
set ignorecase		" Do case insensitive matching
set smartcase		" Do smart case matching
set incsearch		" Incremental search
set autowrite		" Automatically save before commands like :next and :make
set hidden              " Hide buffers when they are abandoned
set mouse=a		" Enable mouse usage (all modes)
set noswapfile
set spell spelllang=en_us
set modeline
set modelines=5

set backspace=eol,start,indent
set whichwrap+=<,>,h,l
set hlsearch
set lazyredraw
set nobackup
set nowb
" Use spaces instead of tabs
" set expandtab
set smarttab
set shiftwidth=8
set tabstop=8
set textwidth=79
set formatoptions=c,q,r,t
set nowrap

" Source a global configuration file if available
if filereadable("/etc/vim/vimrc.local")
  source /etc/vim/vimrc.local
endif

hi clear SpellBad
hi SpellBad cterm=underline,bold
