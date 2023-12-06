#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <time.h>
#include "../include/daemon.h"

#define dump(STR) fprintf(dump, "%s\n", STR)
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1023 * (EVENT_SIZE + 16))
#define ARG_MAX 2097152
bool CHANGE_CFG = false;

void sig_handler(int sig, siginfo_t* info, void* context) {
    CHANGE_CFG = true;
}


void set_pid_file(char* filename) {
    FILE* pid_file;

    pid_file = fopen(filename, "w+");
    if (pid_file)
    {
        fprintf(pid_file, "%u", getpid());
        fclose(pid_file);
    }
}


int monitor_proc(char* cfg_file) {
    Config cfg = {};
    int status = cfg_ctor(&cfg, cfg_file);
    if (status) return status;

    char buffer[BUF_LEN];

    char pathname[256] = "";
    sprintf(pathname, "/proc/%d/cwd", cfg.pid);
    chdir(pathname);

    char cwd[256] = "";
    getcwd(cwd, sizeof(cwd));
    add_watch_dir(cwd, &cfg);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_handler;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGSTOP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while(true) {
        int i = 0;

        if (CHANGE_CFG) {
            cfg_ctor(&cfg, cfg_file);
            CHANGE_CFG = false;
        }

        int length = read(cfg.inotify_fd, buffer, BUF_LEN);

        while(i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    backup_crt_file(pathname, event->name, &cfg);
                } else if (event->mask & IN_DELETE) {
                } else if (event->mask & IN_MODIFY) {
                    backup_mod_file(cwd, event->name, &cfg);
                }
            }
            i += EVENT_SIZE + event->len;
        }
        sleep(cfg.check_time);
    }

    cfg_detor(&cfg);

    return 0;
}

void add_watch_dir(const char* wd, Config* cfg) {
    DIR* dir = opendir(wd);

    if (dir) {
        int inotify_wd = inotify_add_watch(cfg->inotify_fd, wd, IN_MODIFY | IN_CREATE | IN_DELETE);

        cfg->inotify_wds[cfg->inotify_size] = inotify_wd;
        cfg->inotify_size++;

        char path[PATH_MAX];
        char* end_ptr = path;
        struct dirent* e;
        struct stat info;
        strcpy(path, wd);
        end_ptr += strlen(wd);

        while((e = readdir(dir)) != NULL) {
            sprintf(end_ptr, "/");
            end_ptr += 1;
            strcpy(end_ptr, e->d_name);

            if (!stat(path, &info)) {
                if (S_ISDIR(info.st_mode)) {
                    if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
                        end_ptr -= 1;
                        continue;
                    }
                    add_watch_dir(path, cfg);
                }
            }
        }
    }

    return;
}

int backup_crt_file(char* dir_path, char* name, Config* cfg) {
    char file_name[PATH_MAX] = "";
    char cmd[ARG_MAX] = "";
    sprintf(file_name, "%s/found_file", cfg->dump_dir);

    FILE* find_file = fopen(file_name, "w");
    fclose(find_file);
    chdir(dir_path);
    sprintf(cmd, "find -name %s > %s", name, file_name);
    system(cmd);

    find_file = fopen(file_name, "r");
    fscanf(find_file, "%s", file_name);
    fclose(find_file);

    sprintf(cmd, "mkdir %s/%s", cfg->dump_dir, name);
    system(cmd);

    sprintf(cmd, "cp %s %s/%s.backup", file_name, cfg->dump_dir, name);
    system(cmd);

    return 0;
}

int backup_mod_file(char* dir_path, char* name, Config* cfg) {
    char file_name[PATH_MAX] = "";
    char cmd[ARG_MAX] = "";
    sprintf(file_name, "%s/found_file", cfg->dump_dir);

    FILE* find_file = fopen(file_name, "w");
    fclose(find_file);
    chdir(dir_path);
    sprintf(cmd, "find -name %s > %s", name, file_name);
    system(cmd);

    find_file = fopen(file_name, "r");
    fscanf(find_file, "%s", file_name);
    fclose(find_file);

    sprintf(cmd, "mkdir %s/%s", cfg->dump_dir, name);
    system(cmd);

    struct timeval diff_time;
    gettimeofday(&diff_time, NULL);
    char diff_file[ARG_MAX-5000];

    sprintf(diff_file, "%ld_%ld.diff", diff_time.tv_sec, diff_time.tv_usec);

    sprintf(cmd, "diff %s %s/%s.backup > %s/%s/%s", file_name, cfg->dump_dir, name, cfg->dump_dir, name, diff_file);
    system(cmd);

    sprintf(cmd, "cp %s %s/%s.backup", file_name, cfg->dump_dir, name);
    system(cmd);

    return 0;
}

int cfg_ctor(Config* cfg, char* cfg_file_name) {
    FILE* cfg_file = fopen(cfg_file_name, "r");
    if (!cfg_file) {
        printf("Cannot open config file\n");
        return -1;
    }

    int check_time = 0;
    int pid = 0;
    if( fscanf(cfg_file, "%d %d %s", &check_time, &pid, cfg->dump_dir) != 3) return -1;

    cfg->check_time = check_time;
    cfg->pid = pid;

    char cmd[PATH_MAX];
    sprintf(cmd, "mkdir %s", cfg->dump_dir);
    system(cmd);

    cfg->inotify_fd = inotify_init();

    fclose(cfg_file);

    cfg->inotify_capacity = 200;
    cfg->inotify_size = 0;
    cfg->inotify_wds = (int*) calloc(cfg->inotify_capacity, sizeof(int));

    return 0;
}

int cfg_dtor(Config* cfg) {
    for (int i = 0; i < cfg->inotify_size; i++) {
        inotify_rm_watch(cfg->inotify_fd, cfg->inotify_wds[i]);
    }

    close(cfg->inotify_fd);

    free(cfg->inotify_wds);

    return 0;
}