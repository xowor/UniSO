#include <stdlib.h>
#include <stdio.h>
#include "introduction.h"

typedef struct _client {
    //pid

} client;

int msqid;
int pid;
int ppid;

int main(int argc, char** argv){
    pid = getpid();
    ppid = getppid();
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", pid, ppid);

    // scrittura + lettura da file delle risorse: risorse da acquisire + loro quantità + budget
    // registrazione al banditore con messaggio che contiene pid + risorse che gli interessano
    // NB: dimensione del messaggio fissa
    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        msqid = atoi(argv[2]);
        fprintf(stdout, "[client][%d] Using message queue %d\n", getpid(), msqid);
    } else {
        fprintf(stderr, "[client][%d] Error: msqid (-m) argument not valid.\n", getpid());
        return -1;
    }

    fflush(stdout);

    // char*
    introduction* intr = (introduction*) malloc(sizeof(introduction));
    intr->pid = pid;
    // intr->resources =
    msgsnd(msqid, intr, sizeof(introduction) - sizeof(long), 0777);

    // attesa della chiamata dal banditore

    // se banditore comunica acquisizione di risorsa --> scrivere su file di log : risorsa acquisita; quantità; prezzoComplessivo
    // fork --> agenti --> agenti exit --> client
    // lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato
    // exit()
    return 0;
}
