#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std;
vector<string> words;
vector<int> weight1;
vector<int> weight2;
int oldbw;

int collect(string const& i, int w1, int w2) 
{
    istringstream iss(i), iss2(i);
    copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter(words));
    words.insert(words.end(), "\n");
    int a = distance(istream_iterator<string>(iss2), istream_iterator<string>());
    for(int c=0;c<=a;c++) weight1.insert(weight1.end(), w1);
    for(int c=0;c<=a;c++) weight2.insert(weight2.end(), w2);
    return a;
}

void answer(int c) 
{
    string op;
    int bs=0,bw=0,cs,si,searchsize,cc, c2;
    cout << "BRAIN: ";

    for(int iwrd = words.size()-c; iwrd >= 0; iwrd--)
    {
        searchsize = words.size()/4;
        cs = 0; si=searchsize;
        for(int iw = words.size()-2; (iw >= 0 && si!=0); iw--) 
        {   
            cc = iwrd-searchsize+si;
            if (cc>=0) if (words[cc]!="\n" && words[cc]==words[iw]) cs += si*(weight2[cc]-weight1[cc]);
            si--;
        }
        if(cs>=bs && iwrd<(int)words.size()*0.9 && abs(iwrd-oldbw)>((int)words.size()/5)) 
        {bs=cs; bw = iwrd;}
    }

    oldbw=bw;

    if(bw<0)bw=0;
    for(c2=bw; c2< (int)words.size()-1;c2++) if(words[c2]=="\n") {bw=c2+1;break;}

    for(c2=0;(bw+c2) < (int)words.size();c2++)
        if(words[bw+c2]!="\n") op += words[bw+c2] + " "; else break;

    c2=0;
    while(c>=0) 
    {
        if(bw-(++c2)>=0)
        {
            if(weight1[bw-c2]>0) { weight1[bw-c2]--; c--; }
        } else c=-1;

        if(bw+c2>=0)
        {
            if(weight2[bw+c2]>0) { weight2[bw+c2]--; c--; }
        } else c=-1;
    }

    collect(op,3,5);    
    cout << op << "\n  YOU: ";
}

int main() {
    string i;
    oldbw=0;
    do { getline(cin, i); answer(collect(i,5,3)+1);} while(i.compare("exit")!=0);
    return 0;
}
