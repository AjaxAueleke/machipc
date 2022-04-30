#include <stdio.h>
#include <stdlib.h>
#include "mach_process.hpp"

int main(int argc, char const *argv[])
{
    mach_process p(1);

    while (!p.connect(2))
        sleep(1);

    p.send(2, {69});
    return 0;
}
