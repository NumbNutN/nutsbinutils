//to align an address to the specified alignment requirement
#define ROUND(x,align) ((uintptr_t)x & ~((1<<align)-1))
#define MOD(x,align) ((uintptr_t)x % (1<<align))
// ROUND(x,align)   EQU    x - MOD(x,p->p_align)

int size2shift(int size);