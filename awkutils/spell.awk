# Usage: gawk -v word=some_word_to_verify -f spelling.awk [ big.txt [ big2.txt ... ]]
# Gawk version with 15 lines -- 04/13/2008
# Author: tiago (dot) peczenyj (at) gmail (dot) com
#         about.me/peczenyj
# Based on : http://norvig.com/spell-correct.html
# http://pacman.blog.br/blog/2012/12/29/spell-correct-in-gawk/
function edits(w,max,candidates,list,        i,j){
       for(i=0;i<  max ;++i) ++list[substr(w,0,i) substr(w,i+2)] # deletes
       for(i=0;i< max-1;++i) ++list[substr(w,0,i) substr(w,i+2,1) substr(w,i+1,1) substr(w,i+3)] # transposes
       for(i=0;i<  max ;++i) for(j in alpha) ++list[substr(w,0,i) alpha[j] substr(w,i+2)] # replaces
       for(i=0;i<= max ;++i) for(j in alpha) ++list[substr(w,0,i) alpha[j] substr(w,i+1)] # inserts
       for(i in list) if(i in NWORDS) candidates[i] = NWORDS[i] }

function correct(word            ,candidates,i,list,max,temp){
       edits(word,length(word),candidates,list)
       if (!asort(candidates,temp)) for(i in list) edits(i,length(i),candidates)
       return (max = asorti(candidates)) ? candidates[max] : word }

BEGIN{ if (ARGC == 1) ARGV[ARGC++] = "big.txt" # http://norvig.com/big.txt
       while(++i<=length(x="abcdefghijklmnopqrstuvwxyz")) alpha[i]=substr(x,i,1)
       IGNORECASE=RS="[^"x"]+" }

{      ++NWORDS[tolower($1)]   }

END{   print (word in NWORDS) ? word : "correct("word")=> " correct(tolower(word)) }

