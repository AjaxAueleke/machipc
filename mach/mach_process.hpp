#ifndef _MACH_PROCESS_
#define _MACH_PROCESS_

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#include <sys/ipc.h>

#include <queue>
#include <vector>
#include <algorithm>

#include "../include/wrapper.hpp"
#include "../include/msg.hpp"

#include "mach_msg.hpp"
#include "mach_port.hpp"
#include "mach_service.hpp"

#define _MACH_MAX_CLIENTS (50)
#define _ACQUIRE(mutex) (pthread_mutex_lock(&mutex))
#define _RELEASE(mutex) (pthread_mutex_unlock(&mutex))

void *mach_receiving_thread(void *arg);

class mach_process
{
    // central server address:
    const sockaddr_in cserver_addr = {
        AF_INET,
        htons(3333),
        htonl(INADDR_ANY),
    };

    friend void *mach_receiving_thread(void *);

private:
    key_t key;
    mach_port recv_port;
    mach_port service_port;
    pthread_t recv_thread;

    pthread_mutex_t lock;
    std::queue<mach_msg> recv_queue;
    std::vector<mach_service> connected_peers;

public:
    mach_process(key_t _key) : key(_key)
    {
        // random ports:
        mach_port_bind(&recv_port);
        mach_port_bind(&service_port);

        notify_cserver();
        pthread_mutex_init(&lock, 0);
        pthread_create(&recv_thread, NULL, mach_receiving_thread, this);
    }

    ~mach_process()
    {
        mach_service final_req = {
            mach_service_type::exit,
            key,
        };

        writen(
            service_port.sock,
            &final_req,
            sizeof(final_req) //
        );

        close(service_port.sock);
        for (auto &x : connected_peers)
            close(x.port.sock);

        pthread_cancel(recv_thread);
        pthread_join(recv_thread, NULL);
        pthread_mutex_destroy(&lock);
    }

    bool connect(key_t remote_key)
    {
        auto peers = available_peers();

        auto target = std::find_if(
            peers.begin(),
            peers.end(),
            [&](const mach_service &peer)
            { return peer.key == remote_key; } //
        );

        if (target == peers.end())
        {
            fprintf(stderr, "mach_process::connect(%d) = given port is not alive on the central server\n", remote_key);
            return false;
        }

        // connect with process:
        sockaddr_in addr = {
            AF_INET,
            htons(target->port.port),
            htonl(target->port.ip_addr),
            0,
        };

        mach_port send_port;
        mach_port_bind(&send_port);
        Connect(
            send_port.sock,
            &addr,
            sizeof(addr) //
        );

        printf("connecting peer from %d\n", send_port.port);

        target->port = send_port;
        connected_peers.push_back(*target);
        return true;
    }

    bool send(key_t remote_key, mach_msg msg)
    {
        auto target = std::find_if(
            connected_peers.begin(),
            connected_peers.end(),
            [&](const mach_service &peer)
            { return peer.key == remote_key; } //
        );

        if (target == connected_peers.end())
        {
            fprintf(stderr, "mach_process::send(%d) = given port is not connected. use mach_process::connect(%d) to connect with this peer\n", remote_key, remote_key);
            return false;
        }

        writen(
            target->port.sock,
            &msg,
            sizeof(msg) //
        );
        return true;
    }

    bool receive(mach_msg *msg_buffer)
    {
        if (recv_queue.empty())
        {
            msg_buffer = NULL;
            return false;
        }

        _ACQUIRE(lock);
        *msg_buffer = recv_queue.front();
        recv_queue.pop();
        _RELEASE(lock);

        return true;
    }

    void closeCServer()
    {
        mach_service close_req = {
            mach_service_type::closeserver,
        };

        writen(
            service_port.sock,
            &close_req,
            sizeof(close_req) //
        );
    }

private:
    void notify_cserver()
    {
        Connect(
            service_port.sock,
            &cserver_addr,
            sizeof(cserver_addr) //
        );

        mach_service mydata = {
            mach_service_type::init,
            key,
            recv_port,
        };

        writen(
            service_port.sock,
            &mydata,
            sizeof(mydata) //
        );
    }

    std::vector<mach_service> available_peers()
    {
        mach_service peer_req = {
            mach_service_type::peers,
        };

        writen(
            service_port.sock,
            &peer_req,
            sizeof(peer_req) //
        );

        size_t peers_no;
        int err = readn(
            service_port.sock,
            &peers_no,
            sizeof(peers_no) //
        );
        check_server(err);

        std::vector<mach_service> peers;
        for (size_t i = 0; i < peers_no; i++)
        {
            mach_service peer;
            err = readn(
                service_port.sock,
                &peer,
                sizeof(peer) //
            );
            check_server(err);
            peers.push_back(peer);
        }
        return peers;
    }

    void check_server(int err)
    {
        if (err == _MSG_CONNECTION_CLOSED)
        {
            fprintf(stderr, "cserver terminated unexpectedly\n");
            exit(1);
        }
    }
};

void *mach_receiving_thread(void *arg)
{
    mach_process *process = (mach_process *)arg;

    fd_set all_clients;
    std::vector<mach_port> connected_clients;

    sock_t listen_sock = process->recv_port.sock;
    Listen(listen_sock, _MACH_MAX_CLIENTS);

    int maxfd = listen_sock;
    FD_ZERO(&all_clients);
    FD_SET(listen_sock, &all_clients);

    while (true)
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
        // received a msg?:
        for (auto client = connected_clients.begin(); client != connected_clients.end(); client++)
        {
            if (FD_ISSET(client->sock, &readyset))
            {
                struct mach_msg msg;
                int err = readn(
                    client->sock,
                    &msg,
                    sizeof(msg) //
                );

                if (err == _MSG_CONNECTION_CLOSED)
                {
                    close(client->sock);
                    FD_CLR(client->sock, &all_clients);
                    connected_clients.erase(client);

                    fprintf(stderr, "connection closed by remote peer %d\n", client->port);
                }
                else
                {
                    pthread_mutex_lock(&process->lock);
                    process->recv_queue.push(msg);
                    pthread_mutex_unlock(&process->lock);
                }

                if (--nready <= 0) // no more requests?
                    break;
            } // end if
        }     // end for
        pthread_testcancel();
    } // end while

    close(listen_sock);
    for (auto &x : connected_clients)
        close(x.sock);
    return NULL;
}

#endif