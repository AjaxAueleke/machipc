#include <stdio.h>
#include <arpa/inet.h>

in_addr_t pton(const char *ipv4_addr)
{
    in_addr_t result;
    int inet_err = inet_pton(AF_INET, ipv4_addr, &result);

    if (inet_err == 0)
    {
        perror("conv::pton() failed [format error]");
        return -1;
    }

    if (inet_err < 0)
    {
        perror("conv::pton() failed [system error]");
        return -1;
    }

    return result;
}

const char *ntop(in_addr_t numeric_addr, char *buffer)
{
    if (inet_ntop(AF_INET, &numeric_addr, buffer, INET_ADDRSTRLEN) == NULL)
    {
        perror("conv::ntop() failed [conversion error]");
        return NULL;
    }
    return buffer;
}
