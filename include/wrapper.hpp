#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include <netinet/in.h>

typedef int sock_t;

/**
 * @brief Create a new tcp socket descriptor of ipv4 protocol
 * @return socket descriptor on success
 * @throw exit with status 1 on error
 */
sock_t Socket();

/**
 * @brief Open a connection on given socket to peer at given address
 * @param sock a socket descriptor
 * @param remote_addr the peer address structure
 * @param addr_size the length of address structure
 * @return nothing
 * @throw exit with status 1 on error
 */
void Connect(sock_t sock, const sockaddr_in *remote_addr, socklen_t addr_size);

/**
 * @brief Bind the given socket at the given address
 * @param sock a socket descriptor
 * @param local_addr the address structure to bind at
 * @param addr_size the length of address structure
 * @return nothing
 * @throw exit with status 1 on error
 */
void Bind(sock_t sock, const sockaddr_in *local_addr, socklen_t addr_size);

/**
 * @brief Prepare to accept connections on given socket.
 * @param sock a socket descriptor
 * @param max_conn number of connection requests will be queued before further requests are refused
 * @return nothing
 * @throw exit with status 1 on error
 */
void Listen(sock_t sock, int max_conn);

/**
 * @brief Await a connection on given socket. When a connection arrives, open a new socket to communicate with it.
 * @param sock a socket descriptor
 * @param clientaddr pointer to address structure that should receive the connected peer remote address. NULL if not required
 * @param addr_size the size of address structure. 0 if not required
 * @return connected socket descriptor on success
 * @throw exit with status 1 on error
 */
sock_t Accept(sock_t sock, sockaddr_in *clientaddr, socklen_t addr_size);

/**
 * @brief instructs the kernel to wait on multiple descriptors
 * @param maxfdp1 maximum number of descriptors to be tested
 * @param readset pointer to descriptor set to test for reading
 * @return number of ready descriptors. 0 on timeout
 * @throw exit with status 1 on error
 */
int select_readable(int maxfdp1, fd_set *readset);

#endif