// bool convert_immediate_able(int num)
// {
//     for(int i=0;i<=30;i+=2)
//     {
//         int res = ((num >> i) | (num <<(32-i)) ) & ~(0xFF);
//         if( res == 0)
//         {
//             immed = res;
//             rotate = i / 2;
//             return true;
//         }
//     }
//     return false;
// }

// uint32_t convert_immediate(int num){
//     return (uint32_t)num;
// }

bool is_valid_offset(int num)
{
    if(num < 1024 && num > -1024)
    {
        return true;
    }
    else return false;
}