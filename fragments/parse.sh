#!/bin/bash
        awk '
                BEGIN{
                        RS="`""`""`" # Three back quotes mark a code block
                }
                NR%2==0{ # only if we are the middle of a code block
                        if(NR>2)
                                print "EOF\n"$0; # add in heredoc marker
                        else
                                print $0;
                }
                END{
                        print "EOF" # final marker
                }' parse.md |
        sed -e 's/^\([^ E\t].*$\)/pygmentize -l \1 <<EOF/' -e 's/`/\\`/g' | # escape backquote
        bash # pipe to bash or wherever.
