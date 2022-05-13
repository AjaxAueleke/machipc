#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "mach/mach_process.hpp"

int main(int argc, char const *argv[])
{
    mach_process p(2);
    mach_msg msg;
    while (!p.receive(&msg))
        ;

    printf("rec %d\n", msg.id);
    printf("rec %d\n", msg.size);

    return 0;
}
