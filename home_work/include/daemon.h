#include <signal.h>
#include <stdio.h>

typedef struct Config_ {
    int check_time;
    int pid;
    char dump_dir[100];
    int inotify_fd;
    int inotify_size;
    int inotify_capacity;
    int* inotify_wds;
} Config;

void set_pid_file(char* filename);

int monitor_proc(char* filename);

int cfg_ctor(struct Config_* cfg, char* cfg_file_name);

int cfg_detor(Config* cfg);

void sig_handler(int sig, siginfo_t* info, void* context);

void add_watch_dir(const char* wd, Config* cfg);

int backup_mod_file(char* dir_path, char* name, Config* cfg);

int backup_crt_file(char* dir_path, char* name, Config* cfg);

int create_new_backup(char* path, char* name, Config* cfg);