int size2shift(int size){
    int cnt = 0;
    while((size & 1) != 1){
        size >>= 1;
        cnt++;
    }return cnt;
}