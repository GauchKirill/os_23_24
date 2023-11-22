#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
//select -- сложно написал, потоко не безопасный, но самый быстрый

int main()
{
    struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    int max_num_puls_one = 1;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int ret = select(max_num_puls_one, &rfds, NULL, NULL, &tv);

    if (ret)
    {
        if (FD_ISSET(0, &rfds)) // не обнуляет флаги готовности!!! Максимальное кол-во дискрипторов 1024.
        // Все дискрипторы пробегать по циклу, чтобы понять кто готов
        // Потоко не безопасен!
        {
            char str[256];
            read(0 ,str, sizeof(str));
            printf("%s\n", str);
        }
    } else // по timeout
    {
        printf("Timeout\n");
    }

    return 0;
}