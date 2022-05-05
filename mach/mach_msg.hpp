#ifndef MACH_MSG
#define MACH_MSG

typedef uint mach_t;
struct mach_msg
{
    mach_t id;
    char* data;
    __uint128_t size;
};

#endif