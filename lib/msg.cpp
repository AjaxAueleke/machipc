#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define _MSG_INTERRUPTED() (errno == EINTR)
#define _MSG_EOF (0)

typedef int sock_t;

// functions that simplify communicating with other sockets
void writen(sock_t sock, const void *buffer, size_t size)
{
    ssize_t bytes_sent = 0;
    size_t bytes_left = size;
    const char *src_buff = (const char *)buffer;

    while (bytes_left > 0)
    {
        if ((bytes_sent = write(sock, src_buff, bytes_left)) <= 0)
        {
            if (bytes_sent < 0 && _MSG_INTERRUPTED())
                bytes_sent = 0;
            else
            {
                perror("msg::writen() failed");
                return;
            }
        }
        bytes_left -= bytes_sent;
        src_buff += bytes_sent;
    }
}

int readn(sock_t sock, void *buffer, size_t size)
{
    ssize_t bytes_read = 0;
    size_t bytes_left = size;
    char *dest_buff = (char *)buffer;

    while (bytes_left > 0)
    {
        if ((bytes_read = read(sock, dest_buff, bytes_left)) < 0)
        {
            if (_MSG_INTERRUPTED())
                bytes_read = 0;
            else
            {
                perror("msg::readn() failed");
                return (size - bytes_left);
            }
        }
        else if (bytes_read == _MSG_EOF) // connection closed by peer?
            return -1;

        bytes_left -= bytes_read;
        dest_buff += bytes_read;
    }
    return 0;
}
