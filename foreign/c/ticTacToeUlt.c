// Copyright © 2013-2014 Silvain Combis-Schlumberger (schlum@gmail.com)
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// The Software is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the Software.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define GET_N(T,C)			((u_int32_t)((T>>(18*(C)))&0x3FFFF))
#define GET_LX(T3)			((u_int8_t)((T3>>54)&0x3))
#define GET_LY(T3)			((u_int8_t)((T3>>56)&0x3))

#define UPD_N(T,C,V)		T |= (((u_int64_t)((V)&0x3FFFF))<<(18*(C)))
#define UPD_LXY(T3,VX,VY)	T3 = ((T3&0xFC3FFFFFFFFFFFFF)|(((u_int64_t)(VX)&0x3)<<54)|(((u_int64_t)(VY)&0x3)<<56))

#define GET_X(T,L,C)		((u_int8_t)((T>>((L)*6+(C)*2))&0x3))
#define UPD_X(T,L,C,V)		T |= (((u_int32_t)((V)&0x3))<<((L)*6+(C)*2))
#define CLN_X(T,L,C)		T &= ~(0x3<<((L)*6+(C)*2))

#define EVAL_DONE(e)		(e.f>3)

typedef struct {
	int8_t e;
	u_int8_t f;
} EVAL;

static EVAL *EVAL_T = NULL;

typedef struct {
	u_int64_t l[3];
} T;

// 100 -> win
// 101 -> full & draw
// 102 -> no win possible

static u_int8_t calcVal(int tabl[3][3],int val)
{
    u_int8_t res = 0;
    int l,c,k;
	int d = 1;
    // Horiz
    for(c=0;c<3;++c) {
        int cptH = 0;
        for(l=0;l<3&&cptH>=0;++l) {
            if(tabl[l][c]==val)
                ++cptH;
            else if(tabl[l][c]!=0)
                cptH = -1;
        }
        if(cptH==3)
			return 100;
		else if(cptH==2)
            res += 7;
        else if(cptH==1)
            res += 2;
		if(cptH>=0)
			d = 0;
    }
    // Vert
    for(l=0;l<3;++l) {
        int cptV = 0;
        for(c=0;c<3&&cptV>=0;++c) {
            if(tabl[l][c]==val)
                ++cptV;
            else if(tabl[l][c]!=0)
                cptV = -1;
        }
        if(cptV==3)
			return 100;
		else if(cptV==2)
            res += 7;
        else if(cptV==1)
            res += 2;
		if(cptV>=0)
			d = 0;
    }
    // Diag 1
	{
		int cptD1 = 0;
		for(k=0;k<3&&cptD1>=0;++k) {
			if(tabl[k][k]==val)
				++cptD1;
			else if(tabl[k][k]!=0)
				cptD1 = -1;
		}
		if(cptD1==3)
			return 100;
		else if(cptD1==2)
			res += 7;
		else if(cptD1==1)
			res += 2;
		if(cptD1>=0)
			d = 0;
	}
    // Diag 2
	{
		int cptD2 = 0;
		for(k=0;k<3&&cptD2>=0;++k) {
			if(tabl[k][2-k]==val)
				++cptD2;
			else if(tabl[k][2-k]!=0)
				cptD2 = -1;
		}
		if(cptD2==3)
			return 100;
		else if(cptD2==2)
			res += 7;
		else if(cptD2==1)
			res += 2;
		if(cptD2>=0)
			d = 0;
	}
	{
		int f = 1;
		for(l=0;l<3&&f;++l)
			for(c=0;c<3&&f;++c)
				if(tabl[l][c]==0)
					f = 0;
		if(f)
			return 101;
	}
	if(d)
		return 102;
    return res;
}

// EVAL.f
// 0 -> Nothing special
// 1 -> O cannot win
// 2 -> X cannot win
// 3 -> Draw & not full
// 4 -> O wins
// 5 -> X wins
// 6 -> Draw & full
// 7 -> Double win (impossible)

static void initEval()
{
	u_int32_t i;
	EVAL_T = (EVAL*)malloc(0x40000*sizeof(EVAL));
	for(i=0;i<0x40000;++i) {
		u_int8_t c1,c2;
		int tabl[3][3] = { { 0 } };
		int l,c;
		for(l=0;l<3;++l)
			for(c=0;c<3;++c)
				tabl[l][c] = GET_X(i,l,c);
		c1 = calcVal(tabl,1);
		c2 = calcVal(tabl,2);
		if(c1==100&&c2==100) {
			// Double win (impossible)
			EVAL_T[i].e = 0;
			EVAL_T[i].f = 7;
		} else if(c1==100) {
			// O wins
			EVAL_T[i].e = 0;
			EVAL_T[i].f = 4;
		} else if(c2==100) {
			// X wins
			EVAL_T[i].e = 0;
			EVAL_T[i].f = 5;
		} else if(c1==101||c2==101) {
			// Draw & full
			EVAL_T[i].e = 0;
			EVAL_T[i].f = 6;
		} else if(c1==102&&c2==102) {
			// Draw & not full
			EVAL_T[i].e = 0;
			EVAL_T[i].f = 3;
		} else if(c1==102) {
			// O cannot win
			EVAL_T[i].e = c1-c2;
			EVAL_T[i].f = 1;
		} else if(c2==102) {
			// X cannot win
			EVAL_T[i].e = c1-c2;
			EVAL_T[i].f = 2;
		} else {
			// Standard case (diff heuristic)
			EVAL_T[i].e = c1-c2;
			EVAL_T[i].f = 0;
		}
	}
}

typedef struct {
	int32_t evals[3][3];
	int32_t globO;
	int32_t globX;
	EVAL gEvalO;
	EVAL gEvalX;
} HINT;

static void initHint(T tabl,HINT *hint)
{
	int l,c;
	if(hint==NULL)
		return;
	hint->globO = 0;
	hint->globX = 0;
	for(l=0;l<3;++l) {
		for(c=0;c<3;++c) {
			EVAL e = EVAL_T[GET_N(tabl.l[l],c)];
			hint->evals[l][c] = e.e;
			switch(e.f) {
				case 0: // Nothing special
					break;
				case 1: // O cannot win
					UPD_X(hint->globO,l,c,3);
					break;
				case 2: // X cannot win
					UPD_X(hint->globX,l,c,3);
					break;
				case 4: // O wins
					UPD_X(hint->globO,l,c,1);
					UPD_X(hint->globX,l,c,1);
					break;
				case 5: // X wins
					UPD_X(hint->globO,l,c,2);
					UPD_X(hint->globX,l,c,2);
					break;
				case 3: // Draw & not full
				case 6: // Draw & full
				case 7: // Double win (impossible)
				default:
					UPD_X(hint->globO,l,c,3);
					UPD_X(hint->globX,l,c,3);
					break;
			}
		}
	}
	hint->gEvalO = EVAL_T[hint->globO];
	hint->gEvalX = EVAL_T[hint->globX];
}

//  1000000 -> O wins
// -1000000 -> X wins
// 0xFFFFFF -> Draw

static inline int32_t evalue(T tabl,int val,HINT *hint)
{
	int32_t res = 0;
	int l,c;
	if(hint!=NULL) {
		if(hint->gEvalO.f==4)
			return 1000000; // O wins
		if(hint->gEvalX.f==5)
			return -1000000; // X wins
		if(hint->gEvalO.f>=3)
			return 0xFFFFFF; // Draw
		for(l=0;l<3;++l) {
			for(c=0;c<3;++c) {
				if(l==1&&c==1)
					res += 4*hint->evals[l][c];
				else if((l==0||l==2)&&(c==0||c==2))
					res += 3*hint->evals[l][c];
				else
					res += 2*hint->evals[l][c];
			}
		}
		return (5*(val==1?hint->gEvalO.e:hint->gEvalX.e)+res);
	} else {
		u_int32_t globO = 0;
		u_int32_t globX = 0;
		for(l=0;l<3;++l) {
			for(c=0;c<3;++c) {
				EVAL currEv = EVAL_T[GET_N(tabl.l[l],c)];
				switch(currEv.f) {
					case 0: // Nothing special
						if(l==1&&c==1)
							res += 5*currEv.e;
						else if((l==0||l==2)&&(c==0||c==2))
							res += 3*currEv.e;
						else
							res += currEv.e;
						break;
					case 1: // O cannot win
						UPD_X(globO,l,c,3);
						break;
					case 2: // X cannot win
						UPD_X(globX,l,c,3);
						break;
					case 4: // O wins
						UPD_X(globO,l,c,1);
						UPD_X(globX,l,c,1);
						break;
					case 5: // X wins
						UPD_X(globO,l,c,2);
						UPD_X(globX,l,c,2);
						break;
					case 3: // Draw & not full
					case 6: // Draw & full
					case 7: // Double win (impossible)
					default:
						UPD_X(globO,l,c,3);
						UPD_X(globX,l,c,3);
						break;
				}
			}
		}
		EVAL globEvO = EVAL_T[globO];
		EVAL globEvX = EVAL_T[globX];
		if(globEvO.f==4)
			return 1000000; // O wins
		if(globEvX.f==5)
			return -1000000; // X wins
		if(globEvO.f>=3)
			return 0xFFFFFF; // Draw
		return ((val==1?globEvO.e:globEvX.e)*5+res);
	}
}

// <!> UTF-8 drawing boxes chars ; use UTF-8 encoding <!>

static void print(T tabl,int smpl)
{
	int l1,c1,l2,c2;
	int c;
	EVAL e,e2;
	int lX,lY;
	printf("╔");
	for(c=0;c<3;++c) {
		e = EVAL_T[GET_N(tabl.l[0],c)];
		if(smpl&&EVAL_DONE(e))
			printf("═══════════");
		else
			printf("═══╤═══╤═══");
		if(c<2)
			printf("╦");
	}
	printf("╗\n");
	printf("║");
	for(l1=0;l1<3;++l1) {
		for(l2=0;l2<3;++l2) {
			for(c1=0;c1<3;++c1) {
				for(c2=0;c2<3;++c2) {
					e = EVAL_T[GET_N(tabl.l[l1],c1)];
					if(smpl&&EVAL_DONE(e)) {
						if(l2==1&&c2==1) {
							printf(" ");
							if(e.f==4)
								printf("O");
							else if(e.f==5)
								printf("X");
							else
								printf("#");
							printf(" ");
						} else
							printf("   ");
						if(c2<2)
							printf(" ");
					} else {
						u_int8_t tt = GET_X(GET_N(tabl.l[l1],c1),l2,c2);
						if(tt==1)
							printf(" O ");
						else if(tt==2)
							printf(" X ");
						else
							printf("   ");
						if(c2<2)
							printf("│");
					}
				}
				printf("║");
			}
			printf("\n");
			if(l2<2) {
				e = EVAL_T[GET_N(tabl.l[l1],0)];
				if(smpl&&EVAL_DONE(e))
					printf("║");
				else
					printf("╟");
				for(c=0;c<3;++c) {
					e = EVAL_T[GET_N(tabl.l[l1],c)];
					if(smpl&&EVAL_DONE(e))
						printf("           ");
					else
						printf("───┼───┼───");
					if(c<2) {
						e2 = EVAL_T[GET_N(tabl.l[l1],c+1)];
						if(smpl&&EVAL_DONE(e)&&EVAL_DONE(e2))
							printf("║");
						else if(smpl&&EVAL_DONE(e))
							printf("╟");
						else if(smpl&&EVAL_DONE(e2))
							printf("╢");
						else
							printf("╫");
					}
				}
				e = EVAL_T[GET_N(tabl.l[l1],2)];
				if(smpl&&EVAL_DONE(e))
					printf("║\n");
				else
					printf("╢\n");
				printf("║");
			}
		}
		if(l1<2) {
			printf("╠");
			for(c=0;c<3;++c) {
				e = EVAL_T[GET_N(tabl.l[l1],c)];
				e2 = EVAL_T[GET_N(tabl.l[l1+1],c)];
				if(smpl&&EVAL_DONE(e)&&EVAL_DONE(e2))
					printf("═══════════");
				else if(smpl&&EVAL_DONE(e))
					printf("═══╤═══╤═══");
				else if(smpl&&EVAL_DONE(e2))
					printf("═══╧═══╧═══");
				else
					printf("═══╪═══╪═══");
				if(c<2)
					printf("╬");
			}
			printf("╣\n");
			printf("║");
		}
	}
	printf("╚");
	for(c=0;c<3;++c) {
		e = EVAL_T[GET_N(tabl.l[2],c)];
		if(smpl&&EVAL_DONE(e))
			printf("═══════════");
		else
			printf("═══╧═══╧═══");
		if(c<2)
			printf("╩");
	}
	printf("╝\n");
	lX = GET_LX(tabl.l[2]);
	lY = GET_LY(tabl.l[2]);
	if(lX!=3&&lY!=3&&!EVAL_DONE(EVAL_T[GET_N(tabl.l[lX],lY)]))
		printf("Have to play at (%d,%d)\n",lX,lY);
	printf("Debug : tabl[1] = %llu, tabl[2] = %llu, tabl[3] = %llu\n",tabl.l[0],tabl.l[1],tabl.l[2]);
}

static int l1Best = 0;
static int c1Best = 0;
static int l2Best = 0;
static int c2Best = 0;

static void setBest(int l1,int c1,int l2, int c2)
{
    l1Best = l1;
    c1Best = c1;
    l2Best = l2;
    c2Best = c2;
}

static int order[9][2] = {
	{1,1},
	{2,1},
	{2,2},
	{2,0},
	{1,0},
	{0,0},
	{1,2},
	{0,2},
	{0,1},
};

static int MAX_LEV = 0;


static int32_t negamax(T tabl,int val,int lev,int alpha,int beta,HINT *hint)
{
	int best = -10000000;
	int kL;
	u_int8_t lX,lY;
	u_int32_t lastT;
	int32_t eval = (val==1?evalue(tabl,val,hint):-evalue(tabl,val,hint));
	if(eval==0xFFFFFF||eval==-0xFFFFFF)  // Draw
		return 0;
	if(eval==-1000000||eval==1000000)  // Player or AI wins
		return eval;
    if(lev==MAX_LEV)
        return eval;
	lX = GET_LX(tabl.l[2]);
	lY = GET_LY(tabl.l[2]);
	if(lX!=3&&lY!=3)
		lastT = GET_N(tabl.l[lX],lY);
    for(kL=0;kL<9;++kL) {
		int l1,c1,kS;
        l1 = order[kL][0];
        c1 = order[kL][1];
		u_int32_t currT = GET_N(tabl.l[l1],c1);
		if(EVAL_DONE(EVAL_T[currT]))
			continue;
        if(lX!=3&&lY!=3&&(l1!=lX||c1!=lY)&&!EVAL_DONE(EVAL_T[lastT]))
            continue;
        for(kS=0;kS<9;++kS) {
			int32_t hOldEval;
			int hGmod = 0;
			int32_t hOldGlobO,hOldGlobX;
			EVAL hOldGEvalO,hOldGEvalX;
			int l2,c2;
			u_int32_t newT;
            T newTabl;
			l2 = order[kS][0];
            c2 = order[kS][1];
            if(GET_X(currT,l2,c2)!=0)
                continue;
			newT = currT;
			UPD_X(newT,l2,c2,val);
			newTabl = tabl;
			UPD_N(newTabl.l[l1],c1,newT);
			UPD_LXY(newTabl.l[2],l2,c2);
			if(hint!=NULL) {
				EVAL e = EVAL_T[newT];
				hOldEval = hint->evals[l1][c1];
				hint->evals[l1][c1] = e.e;
				if(e.f>0) {
					hGmod = 1;
					hOldGlobO = hint->globO;
					hOldGlobX = hint->globX;
					hOldGEvalO = hint->gEvalO;
					hOldGEvalX = hint->gEvalX;
					switch(e.f) {
						case 1: // O cannot win
							UPD_X(hint->globO,l1,c1,3);
							break;
						case 2: // X cannot win
							UPD_X(hint->globX,l1,c1,3);
							break;
						case 4: // O wins
							UPD_X(hint->globO,l1,c1,1);
							CLN_X(hint->globX,l1,c1); // Can be at 3 if X could not win here -> clup
							UPD_X(hint->globX,l1,c1,1);
							break;
						case 5: // X wins
							CLN_X(hint->globO,l1,c1); // Can be at 3 if O could not win here -> clup
							UPD_X(hint->globO,l1,c1,2);
							UPD_X(hint->globX,l1,c1,2);
							break;
						case 3: // Draw & not full
						case 6: // Draw & full
						case 7: // Double win (impossible)
						default:
							UPD_X(hint->globO,l1,c1,3);
							UPD_X(hint->globX,l1,c1,3);
							break;
					}
					hint->gEvalO = EVAL_T[hint->globO];
					hint->gEvalX = EVAL_T[hint->globX];
				}
			}
			int32_t calc = -negamax(newTabl,3-val,lev+1,-beta,-alpha,hint);
			if(hint!=NULL) {
				hint->evals[l1][c1] = hOldEval;
				if(hGmod) {
					hint->globO = hOldGlobO;
					hint->globX = hOldGlobX;
					hint->gEvalO = hOldGEvalO;
					hint->gEvalX = hOldGEvalX;
				}
			}
            if(lev==0) {
                printf(".");
                fflush(stdout);
            }
			// Negamax params update & pruning
			if(calc>best) {
				if(lev==0)
					setBest(l1,c1,l2,c2);
				if(calc==1000000)
					return 1000000;
				best = calc;
				if(best>alpha) {
					alpha = best;
					if(alpha>=beta)
						return best;
				}
			}
        }
    }
    return best;
}

static int playAt(T *tabl,int val,int l1,int c1, int l2, int c2)
{
	int lX,lY;
	u_int32_t lastT,currT;
	EVAL eval;
	int32_t gEval;
    if(tabl==NULL||l1<0||l1>2||c1<0||c1>2||l2<0||l2>2||c2<0||c2>2) {
        printf("Bad params...\n");
        return 2;
    }
	lX = GET_LX(tabl->l[2]);
	lY = GET_LY(tabl->l[2]);
	if(lX!=3&&lY!=3)
		lastT = GET_N(tabl->l[lX],lY);
	currT = GET_N(tabl->l[l1],c1);
	if((lX!=3&&lY!=3&&(l1!=lX||c1!=lY)&&!EVAL_DONE(EVAL_T[lastT]))||EVAL_DONE(EVAL_T[currT])||GET_X(currT,l2,c2)!=0) {
        printf("Cannot play there...\n");
        return 2;
    }
	UPD_X(currT,l2,c2,val);
	UPD_N(tabl->l[l1],c1,currT);
	UPD_LXY(tabl->l[2],l2,c2);
	eval = EVAL_T[currT];
	switch(eval.f) {
		case 0: // Nothing special
			break;
		case 1: // O cannot win
			printf("Part at l1 = %d, c1 = %d won't be for me\n",l1, c1);
			break;
		case 2: // X cannot win
			printf("Part at l1 = %d, c1 = %d won't be for you\n",l1, c1);
			break;
		case 4: // O wins
			printf("Gives me victory at l1 = %d, c1 = %d\n",l1, c1);
			break;
		case 5: // X wins
			printf("Gives you victory at l1 = %d, c1 = %d\n",l1, c1);
			break;
		case 3: // Draw & not full
			printf("Part at l1 = %d, c1 = %d is a draw (not full)\n",l1, c1);
			break;
		case 6: // Draw & full
			printf("Part at l1 = %d, c1 = %d is full with a draw\n",l1, c1);
			break;
		case 7: // Double win (impossible)
			printf("Part at l1 = %d, c1 = %d is in a wrong state (double win) !!\n",l1, c1);
			break;
		default:
			printf("Part at l1 = %d, c1 = %d is in a wrong state (%d) !!\n",l1, c1, (int)eval.f);
			break;
	}
	gEval = evalue(*tabl,val,NULL);
	if(gEval==1000000) {
		printf("I won !\n");
		return 1;
	} else if(gEval==-1000000) {
		printf("You won !\n");
		return 1;
	} else if(gEval==0xFFFFFF) {
		printf("It is a draw...\n");
		return 1;
	}
    return 0;
}

static int playHuman(T *tabl)
{
    int ok = 0;
    int l1, c1, l2, c2;
	int rep = 0;
	if(tabl==NULL)
		return 1;
    while(!ok) {
        printf("Please give l1, c1, l2, c2:\n");
        scanf("%1d %1d %1d %1d",&l1,&c1,&l2,&c2);
        scanf ("%*[^\n]");
        getchar();
		rep = playAt(tabl,2,l1,c1,l2,c2);
		if(rep<2)
			ok = 1;
    }
    printf("You are playing at l1 = %d, c1 = %d, l2 = %d, c2 = %d\n",l1,c1,l2,c2);
	print(*tabl,1);
    return rep;
}

static int playManual(T *tabl,int val)
{
    int l1, c1, l2, c2;
	int rep;
	if(tabl==NULL)
		return 1;
	printf("Please give l1, c1, l2, c2:\n");
	scanf("%1d %1d %1d %1d",&l1,&c1,&l2,&c2);
	scanf ("%*[^\n]");
	getchar();
	rep = playAt(tabl,val,l1,c1,l2,c2);
	if(rep<2) {
		printf("You are playing %s at l1 = %d, c1 = %d, l2 = %d, c2 = %d\n",val==1?"O":"X",l1,c1,l2,c2);
		print(*tabl,1);
	}
	UPD_LXY(tabl->l[2],3,3);
	return rep;
}

#include <time.h>

static int playComputer(T *tabl)
{
    int res = 0;
    int t = 0;
	int rep = 0;
	HINT hint;
	if(tabl==NULL)
		return 1;
	initHint(*tabl,&hint);
    MAX_LEV = 6;
    for(;MAX_LEV<=20&&t<10*CLOCKS_PER_SEC&&res!=1000000&&res!=-1000000;++MAX_LEV) {
        clock_t tStart = clock();
        printf("Depth = %2d   ",MAX_LEV);
        fflush(stdout);
        res = negamax(*tabl,1,0,-10000000,10000000,&hint);
        t = (clock()-tStart);
        printf("   %6d second(s) -> res = %d\n",t/CLOCKS_PER_SEC,res);
    }
    if(res==1000000)
        printf("I will win for sure now...\n");
    else if(res==-1000000) {
        printf("I feel it very bad...\n");
        MAX_LEV -= 2;
        for(;res==-1000000;--MAX_LEV) {
			clock_t tStart = clock();
			printf("Depth = %2d   ",MAX_LEV);
			fflush(stdout);
            res = negamax(*tabl,1,0,-10000000,10000000,&hint);
			t = (clock()-tStart);
			printf("   %6d second(s) -> res = %d\n",t/CLOCKS_PER_SEC,res);
		}
    } else if(res==0)
		printf("I suspect there may be a draw...\n");
    printf("I will play at l1 = %d, c1 = %d, l2 = %d, c2 = %d\n",l1Best,c1Best,l2Best,c2Best);
    rep = playAt(tabl,1,l1Best,c1Best,l2Best,c2Best);
	print(*tabl,1);
    return rep;
}

int main(int argc,char *argv[])
{
	int ok = 0;
	T tabl;
	memset(&tabl,'\0',sizeof(tabl));
	UPD_LXY(tabl.l[2],3,3);
	initEval();
	print(tabl,1);
	//ok = playComputer(&tabl); // For computer start
	while(!ok) {
        ok = playHuman(&tabl);
        if(!ok)
            ok = playComputer(&tabl);
    }
    return 0;
}
