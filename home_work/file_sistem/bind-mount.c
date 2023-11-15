#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <linux/limits.h>

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Pass target path\n");
        return 1;
    }

    char tgt_path[PATH_MAX];
    const char *src = "/home/kirill";
    strcpy(tgt_path, argv[1]);
    int res = mount(src, tgt_path, NULL, MS_BIND | MS_REC, NULL);
    if (res == 0)
    {
        printf("mounted at: %s\n", tgt_path);
        printf("Print <key> to unmount...");
        getchar();
        umount(tgt_path);
    }
    else
    {
        perror("mount");
    }
    
    return 0;
}