#ifndef MESSAGE_H
#define MESSAGE_H

#define _MSG_CONNECTION_CLOSED (-1)

typedef int sock_t;

/**
 * @brief write given amount of bytes on given socket
 * @param sock a socket descriptor
 * @param buffer pointer to source buffer
 * @param size number of bytes to send
 * @return nothing
 * @throw error on internal error
 */
void writen(sock_t sock, const void *buffer, size_t size);
/**
 * @brief read given amount of bytes from given socket
 * @param sock a socket descriptor
 * @param buffer pointer to destination buffer
 * @param size number of bytes to read
 * @return 0 on succes; -1 if connection closed by peer
 * @throw return number of bytes sent on error
 */
int readn(sock_t sock, void *buffer, size_t size);

#endif
