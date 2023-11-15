#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

int main()
{
    int pid = getpid();

    char fn[PATH_MAX];
    sprintf(fn, "/proc/%d/stat", pid);
    FILE* f = fopen(fn, "r");
    if (f == NULL)
        perror("fopen");
    char cmd[4096];
    char state;
    int ppid;
    int prefix;
    fscanf(f, "%d %s %c %d", &prefix, cmd, &state, &ppid);
    printf("pid %d, cmd: %s, state: %c, ppid:%d\n", pid, cmd, state, ppid);
    return 0;
}