#include <stdio.h>

#define SMALL_SIZE  100
#define NORMAL_SIZE 4096
#define BIG_SIZE    8192

int main()
{
    FILE* small     = fopen("input_file_4_less.txt", "w");
    FILE* normal    = fopen("input_file_4.txt", "w");
    FILE* big       = fopen("input_file_4_more.txt", "w");

    for (size_t i = 0; i < SMALL_SIZE; i++)
    {
        fputc('a', small);
        fputc('b', normal);
        fputc('c', big);
    }
    for (size_t i = SMALL_SIZE; i < NORMAL_SIZE; i++)
    {
        fputc('b', normal);
        fputc('c', big);
    }
    for (size_t i = NORMAL_SIZE; i < BIG_SIZE; i++)
    {
        fputc('c', big);
    }
    fclose(small);
    fclose(normal);
    fclose(big);
    
    return 0;
}