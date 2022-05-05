#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <vector>
#include <algorithm>

#include "../include/wrapper.hpp"
#include "../include/msg.hpp"

#include "mach_port.hpp"
#include "mach_service.hpp"

#define MACH_MAX_CLIENTS (50)

int main()
{
    fd_set all_clients;
    mach_port listen_port;
    std::vector<mach_port> connected_clients;
    std::vector<mach_service> stored_clients;

    mach_port_bind(&listen_port, 3333);
    Listen(listen_port.sock, MACH_MAX_CLIENTS);

    sock_t listen_sock = listen_port.sock;
    int maxfd = listen_sock;

    FD_ZERO(&all_clients);
    FD_SET(listen_sock, &all_clients);

    printf("listening at %d on localhost\n", listen_port.port);

    bool run_server = true;
    while (run_server)
    {
        fd_set readyset = all_clients; // work on copy
        int nready = select_readable(maxfd + 1, &readyset);

        if (FD_ISSET(listen_sock, &readyset)) // connection detected?
        {
            sockaddr_in client_addr;
            sock_t connected_sock = Accept(listen_sock, &client_addr, sizeof(client_addr));
            FD_SET(connected_sock, &all_clients);

            mach_port client_port = {
                connected_sock,
                htons(client_addr.sin_port),
                htonl(client_addr.sin_addr.s_addr),
            };
            connected_clients.push_back(client_port);

            printf("connection from %d\n", client_port.port);

            if (connected_sock > maxfd)
                maxfd = connected_sock;
            if (--nready <= 0) // no more requests?
                continue;
        }
        // received a request
        for (auto client = connected_clients.begin(); client != connected_clients.end(); client++)
        {
            if (FD_ISSET(client->sock, &readyset))
            {
                struct mach_service req;
                printf("Client's socket [MACH SERVER] : %d\n", client->sock);
                int err = readn(
                    client->sock,
                    &req,
                    sizeof(req) //
                );

                if (err == _MSG_CONNECTION_CLOSED)
                {
                    close(client->sock);
                    FD_CLR(client->sock, &all_clients);
                    connected_clients.erase(client);
                    printf("disconnected client %d\n", client->port);
                    break;
                }

                switch (req.type)
                {
                // client has notified?
                case mach_service_type::init:
                {
                    stored_clients.push_back(req);
                    break;
                }
                // client wants available peers?
                case mach_service_type::peers:
                {
                    size_t peer_no = stored_clients.size();
                    writen(
                        client->sock,
                        &peer_no,
                        sizeof(peer_no) //
                    );

                    for (auto &c : stored_clients)
                    {
                        writen(
                            client->sock,
                            &c,
                            sizeof(c) //
                        );
                        printf("- sending %d %d\n", c.key, c.port.port);
                    }
                    break;
                }
                // client is exiting?
                case mach_service_type::exit:
                {
                    for (auto it = stored_clients.begin(); it != stored_clients.end(); it++)
                    {
                        if (it->key == req.key)
                        {
                            stored_clients.erase(it);
                            break;
                        }
                    }
                    break;
                }
                // close the central server?
                case mach_service_type::closeserver:
                {
                    run_server = false;
                    printf("- closing server\n");
                    break;
                }
                }                  // end switch
                if (--nready <= 0) // no more requests?
                    break;
            } // end if
            if (run_server == false)
                break;
        } // end for
    }     // end while

    printf("exiting\n");
    close(listen_sock);
    return 0;
}
