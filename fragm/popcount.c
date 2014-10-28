/*howerj*/
unsigned popcount(unsigned int count){
        unsigned int x = 0;
        do x += count & 1; while(count>>=1);
        return x;
}
