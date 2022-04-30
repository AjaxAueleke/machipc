#include <stdio.h>
#include <stdlib.h>
#include "mach_process.hpp"

int main(int argc, char const *argv[])
{
    mach_process p(2);

    mach_msg msg;
    while (!p.receive(&msg))
        ;

    printf("rec %d\n", msg.data);

    return 0;
}
