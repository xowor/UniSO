#include <stdio.h>

typedef struct _client {
    //pid

} client;

int main(){
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", getpid(), getppid());
    fflush(stdout);
    
    // fork --> agenti --> agenti exit --> client
    // lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato
    // exit()
    return 0;
}
