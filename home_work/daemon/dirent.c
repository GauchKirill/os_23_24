#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <linux/limits.h>
#include <string.h>

void list_dir(const char* wd, int offset)
{
    DIR *dir = opendir(wd);
    if (dir)
    {
        char *last_file_name;
        struct dirent *e;
        struct stat info;
        int cnt = offset;

        while ((e = readdir(dir)) != NULL)
        {
            printf("%*s", cnt*4, "");
            printf("%s\n", e->d_name);  
            
            if (e->d_type == DT_DIR)
            {
                if ((strcmp(e->d_name, ".") == 0) || (strcmp(e->d_name, "..") == 0))
                    continue;
                
                char path[PATH_MAX];
                strcpy(path, wd);
                list_dir(strcat(strcat(path, "/"), e->d_name), cnt+1);
            }   
        }
    } 
    return;
}

int main()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd , sizeof(cwd)) != NULL) //return name dir (current work directory)
    {
        printf("listing dir: %s\n", cwd);
        list_dir(cwd, 0);
    }
    return 0;
}