#include <stdio.h>
#include <unistd.h>

int main()
{
    //srace ./... - как выделялась память
    //brk(ptr) -- сдвигает PROGRAM BREAK на ptr, ptr == NULL -> возвращает текущий
    void *currBreak = sbrk(0); //возвращает старый
    printf("First brk %p\n", currBreak);
    currBreak = sbrk(0);
    printf("Current brk %p\n", currBreak);
    return 0;
}