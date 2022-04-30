#include <stdio.h>
#include "mach_process.hpp"

int main(int argc, char const *argv[])
{
    mach_process p(1);

    p.closeCServer();

    return 0;
}
