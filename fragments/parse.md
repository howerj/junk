## parse.md

The following bash script should parse out markdown literal blocks and
colorize them, it is probably very buggy. But it does parse and colorize
itself.

```bash
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
```

## More examples

This is an another example but in C.

```c
        #include <stdio.h>
        int main(void){
                return printf("Hello, World!\n"), 0;
        }
```

And yet another C example:

```c
        #include <stdio.h>
        int main(int argc, char **argv){
                while(EOF!=puts(argc>1?argv[1]:"y"));
                return 0;
        }
```

That is all!

