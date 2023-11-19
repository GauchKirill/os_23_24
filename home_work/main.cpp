#include "daemon.h"

int main(int argc, char** argv)
{
    daemon(argc, argv);
    return 0;
}