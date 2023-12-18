#ifndef MASTER_H
#define MASTER_H

#define PORT 8123
#define MAXLINE 1024 
#define BUF_SIZE 4096
#define IP_LEN 15
#define MAX_SLAVES 10

const int WAIT_TIME = 7;

struct Task {
    double x1;
    double x2;
    double y1;
    double y2;
    long long num_of_points;
};

struct Slave {
    struct Task task;
    char ip[IP_LEN];
    int num_of_cores;
    int socket;
};

struct Server {
    struct Slave slaves[MAX_SLAVES];
    int num_of_slaves;
};

#endif