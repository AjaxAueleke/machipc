#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

typedef int sock_t;

sock_t Socket()
{
    sock_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket() failed");
        exit(1);
    }
    return sock;
}

void Connect(sock_t sock, const sockaddr_in *remote_addr, socklen_t addr_size)
{
    if (connect(sock, (sockaddr *)remote_addr, addr_size) < 0)
    {
        perror("Connect() failed");
        exit(1);
    }
}

void Bind(sock_t sock, const sockaddr_in *local_addr, socklen_t addr_size)
{
    if (bind(sock, (sockaddr *)local_addr, addr_size) < 0)
    {
        perror("Bind()");
        exit(1);
    }
}

void Listen(sock_t sock, int max_conn)
{
    if (listen(sock, max_conn) < 0)
    {
        perror("Listen() failed");
        exit(1);
    }
}

sock_t Accept(sock_t sock, sockaddr_in *clientaddr, socklen_t addr_size)
{
    sock_t connfd = accept(sock, (sockaddr *)clientaddr, &addr_size);
    if (connfd < 0)
    {
        perror("Accept() failed");
        exit(1);
    }
    return connfd;
}

int select_readable(int maxfdp1, fd_set *readset, time_t timeout_sec)
{
    int nready;
    struct timeval timeout;

    if (timeout_sec == 0)
        nready = select(maxfdp1, readset, NULL, NULL, NULL);
    else
    {
        timeout.tv_sec = timeout_sec;    
        nready = select(maxfdp1, readset, NULL, NULL, &timeout);
    }
    
    if (nready < 0)
    {
        perror("select_readable() failed");
        exit(1);
    }
    return nready;
}
