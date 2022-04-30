#ifndef _MACH_SERVICE_
#define _MACH_SERVICE_

#include "mach_port.hpp"

enum class mach_service_type
{
    init,
    peers,
    exit,
    closeserver,
};

struct mach_service
{
    mach_service_type type;
    key_t key;
    mach_port port;
};

#endif