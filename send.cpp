#include <stdio.h>
#include <stdlib.h>
#include "mach/mach_process.hpp"
#include <sys/shm.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    mach_process p(1);
    while (!p.connect(2))
        sleep(1);

    mach_msg msg;
    msg.id = 69;
    msg.size = sizeof(int);

    p.send(2, msg);

    return 0;
}
