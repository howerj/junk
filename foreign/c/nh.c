/*https://codegolf.stackexchange.com/questions/52547/minimal-nethack*/
#define o ({for(int t=0;t<80;++t)t%16||putchar(10),putchar(t^p?t^a?46:34:64);})
p;main(a){for(a=1+time(0)%79;p^a;o,p+=(int[]){-16*(p>15),16*(p<64),-!!(p%16),p%16<15}[3&getchar()/2]);}
