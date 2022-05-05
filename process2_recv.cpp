#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "mach/mach_process.hpp"

int main(int argc, char const *argv[])
{
    mach_process p(2);

    mach_msg msg;

    while (!p.receive(&msg))
        ;

    
    key_t key = ftok("shm",msg.id);

    if (key == -1)
    {
        perror("ftok[rec] : ");
    
    }

    int shmid = shmget(key,msg.size,0666|IPC_CREAT);
    char *shm = (char *)shmat(shmid, NULL, 0);
    printf("rec %s\n", shm);

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
