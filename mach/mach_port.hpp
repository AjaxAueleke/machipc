#ifndef _MACH_PORT_
#define _MACH_PORT_

#include <strings.h>
#include "include/wrapper.hpp"

struct mach_port
{
    sock_t sock;
    in_port_t port;
    in_addr_t ip_addr;
};

void mach_port_bind(mach_port *port, unsigned port_no = 0)
{
    port->sock = Socket();

    sockaddr_in x = {
        AF_INET,
        htons(port_no),
        htonl(INADDR_ANY),
    };
    Bind(port->sock, &x, sizeof(x));

    bzero(&x, sizeof(x));
    socklen_t x_size = sizeof(x);
    getsockname(port->sock, (sockaddr *)&x, &x_size);

    port->port = ntohs(x.sin_port);
    port->ip_addr = ntohs(x.sin_addr.s_addr);
}

#endif