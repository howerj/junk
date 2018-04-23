#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

using namespace std;
vector<string> words;

class c_wordgroup {
public:
    int                 sorting[1000];
    string              words[5];

    void resetsort(int ns)
    {
        for(int i = 0; i<1000;i++)
            sorting[i] = ns;
    }
};

class c_wglist {
public: int                 c;
     vector<c_wordgroup>    wg;
     vector<int>            historywg;



     int wgexists(c_wordgroup nwg)
     {
         vector<c_wordgroup>::iterator it;
         int cc = 0;
         for (it = wg.begin(); it != wg.end(); ++it) {

             // Check if words is the same
             if(it->words[0] == nwg.words[0])
                 if(it->words[1] == nwg.words[1])
                     if(it->words[2] == nwg.words[2])
                        return cc;

             cc++;
         }
         return -1;
     }

     int getbestnext(int lastwg)
     {
         vector<c_wordgroup>::iterator  it;
         int cc = 0;

         int bv = -1;
         int bwg = 0;

         for (it = wg.begin(); it != wg.end(); ++it) {

             bool cont = false;
             for (int iti = 0; iti<((int)historywg.size()/50+5);iti++)
                 if((int)wgl.historywg.size()-1-iti>=0)
                    if (cc==wgl.historywg[(int)wgl.historywg.size()-1-iti])
                        cont = true;

             if(cont==true) {cc++;continue;};

             int cv = 100000000;

             // Check if words is the same
             if(it->words[0] == wgl.wg[lastwg].words[1])
             {
                 for(int si=0;si<1000;si++)
                     if ((int)wgl.historywg.size()-1-si>=0)
                     {
                            int tmpwg = wgl.historywg[(int)wgl.historywg.size()-1-si];
                            cv -= abs(it->sorting[si]-wgl.wg[tmpwg].sorting[si])/(si+1);
                     }
             } else cv -= 1000 * wgl.c/2;

             if(it->words[1] == wgl.wg[lastwg].words[2])
             {
                 for(int si=0;si<1000;si++)
                    if ((int)wgl.historywg.size()-1-si>=0)
                     {
                            int tmpwg = wgl.historywg[(int)wgl.historywg.size()-1-si];
                            cv -= abs(it->sorting[si]-wgl.wg[tmpwg].sorting[si])/(si+1);
                     }
             } else cv -= 1000 * wgl.c/2;

            if(bv == -1 || cv > bv)
            {
                bwg=cc;
                bv = cv;
            }
            cc++;
         }
         return bwg;
     }
} wgl;

void answer2() 
{
    vector<string> lastwords;
    lastwords.insert(lastwords.end(), words[words.size()-3]);
    lastwords.insert(lastwords.end(), words[words.size()-2]);
    lastwords.insert(lastwords.end(), words[words.size()-1]);

    int bestnextwg;

    cout << "\n Mindless:   ";
    for(int ai=0;ai<20;ai++)
    {
        bestnextwg=wgl.getbestnext(wgl.historywg[(int)wgl.historywg.size()-1]);

        if(wgl.wg[bestnextwg].words[2]=="[NL]")
            ai=20;
        else
            cout << wgl.wg[bestnextwg].words[2] << " ";
        wgl.historywg.insert(wgl.historywg.end(), bestnextwg);
    }

}

int collect2(string const& i) 
{
    istringstream iss(i), iss2(i), iss3(i);
    vector<string> nwords;
    nwords.insert(nwords.end(), words[words.size()-2]);
    nwords.insert(nwords.end(), words[words.size()-1]);

    copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter(words));

    copy(istream_iterator<string>(iss3),
             istream_iterator<string>(),
             back_inserter(nwords));

    int a = distance(istream_iterator<string>(iss2), istream_iterator<string>());

    c_wordgroup nwg;

    for (int c=0;c<a;c++)
    {
        nwg.resetsort(wgl.c+1);
        nwg.words[0] = nwords[0+c];
        nwg.words[1] = nwords[1+c];
        nwg.words[2] = nwords[2+c];

        int wge=wgl.wgexists(nwg);

        if(wge>=0) {
            for(int hi=0; hi<1000; hi++)
                if(((int)wgl.historywg.size()-hi-1)>=0)
                {   
                    int iwg = wgl.historywg[(int)wgl.historywg.size()-hi-1];
                    wgl.wg[wge].sorting[hi] = (wgl.wg[wge].sorting[hi] + wgl.wg[iwg].sorting[hi])/2;
                }

            wgl.historywg.insert(wgl.historywg.end(), wge);

        } else {
            wgl.c++;
            // adjust history wordgroup sortings.
            for(int hi=0; hi<1000; hi++)
                if(((int)wgl.historywg.size()-hi-1)>=0)
                {   
                    int iwg = wgl.historywg[(int)wgl.historywg.size()-hi-1];
                    wgl.wg[iwg].sorting[hi]+=10;
                    nwg.sorting[hi]=wgl.wg[iwg].sorting[hi];
                }

            wgl.wg.insert(wgl.wg.end(), nwg);
            wgl.historywg.insert(wgl.historywg.end(), wgl.c);

        }
    }
    return a;
}

int main() {
    string i;
    wgl.c = 0;
    c_wordgroup nwg;
    nwg.resetsort(0);
    for(int i =0;i<3;i++) 
        {
            words.insert(words.end(), "[NL]");
            wgl.historywg.insert(wgl.historywg.end(), 0);
        }

    wgl.wg.insert(wgl.wg.end(), nwg);

    do { 
        cout << "\n      YOU:   ";
        getline(cin, i);
        collect2(i + " [NL]");
        answer2();
    } while (i.compare("exit")!=0);

    return 0;
}
