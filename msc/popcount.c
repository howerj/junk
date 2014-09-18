/*howerj*/
unsigned popcount(unsigned int count){
        unsigned int x = 0;
        do{
                if(count&1)
                        x++;
        } while(count>>=1);
        return x;
}
