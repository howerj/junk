// http://frankforce.com/?p=5826
#include <windows.h>//BUSINESS CARD FPS
#include <cmath>    // By Frank Force
#define A return    //  2019 ><{{{*> 
typedef double O;typedef int e;struct f
{f(O h,O y){a=h;o=y;}f(O g){a=sin(g);o=
cos(g);}f operator+(f h){A f(a+h.a,o+h.
o);}O a,o;f operator*(O h){A f(h*a,h*o)
;}};O b(e y){A O(1&GetKeyState(y)>>16);
}const e i=1<<19,s=97;e h[i]={0};e l(f 
r){A h[e(r.o)*s+e(r.a)];}e main(){e a,L
;O u=7;for(L=9215;--L>s;)h[L+(rand()&1?
1:s)]=(2+L)%s>2;h[s+2]=1;f h(2,1);char 
F[i],N[]="#+. \xDB\xB2\xB1\xB0";void*_=
CreateConsoleScreenBuffer(3<<30,0,0,1,0
);SetConsoleActiveScreenBuffer(_);COORD
H={0};CONSOLE_SCREEN_BUFFER_INFO T;_:G\
etConsoleScreenBufferInfo(_,&T);DWORD Y
;Sleep(16);f o=f(u+=(b(68)-b(65))/30)*(
b(87)-b(83))*.1;h=h+f(o.a*l(h+f(o.a,0))
,o.o*l(h+f(0,o.o)));e I=T.dwSize.X,S=T.
dwSize.Y;for(L=I;L--;){f n(u+atan2(L/O(
I)-.5,1)),Y(e(h.a),e(h.o));O s=n.a>0?1:
-1,H=n.o>0?1:-1,E=abs(1/n.a),u=0,o=abs(
1/n.o),y=E*s*(Y.a-h.a+(s>0)),W=H*o*((H>
0)+Y.o-h.o);f i(s,H);while(!u){y<W?y+=E
,u=1,Y.a+=i.a:(W+=o,Y.o+=i.o,u=2);u*=!l
(Y);}O d=u<2?(Y.a-h.a-i.a/2+.5)/n.a:(Y.
o-h.o-i.o/2+.5)/n.o,T=S/2-S/(d+.1),p=(u
<2?n.o*d+h.o:h.a+d*n.a)+.1;for(a=S;a--;
)F[a*I+L]=e(a>S-T?7-8*a/S:T>a?8*a/S:d>9
?7:(.2>p-e(p))+d/3+4)[N];}WriteConsole\
OutputCharacter(_,F,I*S,H,&Y);goto _;}
