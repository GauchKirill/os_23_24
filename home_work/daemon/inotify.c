#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_SZ (1024 * (EVENT_SIZE + 16))

int main()
{
    char buf[BUF_SZ];
    int fd;
    int wd;
    int len;
    int i = 0;
    fd = inotify_init();
    if (fd < 0)
        perror("Inotify_init");

    wd = inotify_add_watch(fd, ".", IN_MODIFY | IN_CREATE | IN_DELETE);
    while (1)
    {
        i = 0;
        len = read(fd, buf, BUF_SZ);
        while (i < len)
        {
            struct inotify_event *ev = (struct inotify_event*) &buf[i];
            if (ev->len)
            {
                if (ev->mask & IN_CREATE)
                    printf("FILE %s created\n", ev->name);
                
                if (ev->mask & IN_MODIFY)
                    printf("FILE %s modified\n", ev->name);

                if (ev->mask & IN_DELETE)
                    printf("FILE %s deleted\n", ev->name);

            }
            i += EVENT_SIZE + ev->len;
        }
    }
    
    inotify_rm_watch(fd, wd);
    close(fd);
    
    
    return 0;
}

