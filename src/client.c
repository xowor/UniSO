#include <stdio.h>

typedef struct _client {
    //pid

} client;

int msqid;

int main(int argc, char** argv){
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", getpid(), getppid());

    if (argc >= 2 && strcmp(argv[0], "-m") == 0){
        msqid = atoi(argv[1]);
        fprintf(stdout, "[client][%d] Using message queue %d\n", getpid(), msqid);
    } else {
        fprintf(stderr, "[client] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

    fflush(stdout);

    // fork --> agenti --> agenti exit --> client
    // lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato
    // exit()
    return 0;
}
