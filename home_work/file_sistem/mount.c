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
    const char *src = "none";
    const char *fstype = "tmpfs";
    const char *opts = "mode=0700,uid=65534";
    strcpy(tgt_path, argv[1]);
    int res = mount(src, tgt_path, fstype, 0, opts);
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