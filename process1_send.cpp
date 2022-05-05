#include <stdio.h>
#include <stdlib.h>
#include "mach/mach_process.hpp"
#include <sys/shm.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char data[1024] = {0};
    printf( "Write Data : \n");
    scanf("%[^\n]s",data);
    mach_process p(1);

    while (!p.connect(2))
        sleep(1);

    mach_msg msg;
    msg.id = 69;
    printf("Data written in memory: %s\n", data);

    key_t key = ftok("shm", msg.id);
    if (key == -1)
    {
        perror("ftok[sendprc] : ");
    }
    int shmid = shmget(key,1024,0666|IPC_CREAT);

    if (shmid == -1)
    {
        perror("shmget[sendprc] : ");
    }

    char *shm = (char *)shmat(shmid, NULL, 0);
    if (shm == (char *)-1)
    {
        perror("shmat[sendprc] : ");
    }
 
    strcpy(shm, data);
    msg.data = shm;
    msg.size = strlen(data);

    p.send(2, msg);

    if(shmdt(shm)) {
        perror("shmdt[sendprc] : ");
    }
    return 0;
}
