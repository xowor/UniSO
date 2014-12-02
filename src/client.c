#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "introduction.h"

typedef struct _client {
    //pid

} client;

int msqid;
int client_num;
int pid;
int ppid;
char* required_resources[MAX_REQUIRE_RESOURCES];
int required_resources_length = 0;


void load_resources(){
    FILE* resources;
    char line[64];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0;
    char filename[1024];
    sprintf(filename, "../resources/clients/%d.txt", client_num);
    printf("%s\n", filename);
    resources = fopen(filename, "r");
    if( resources != NULL ){
        /* Reads each line from file */
        while( fgets(line, 64, resources) != NULL ){
            token = strtok(line, ";");
            i = 0;
            name = (char*) malloc(64);
            while( token ){
                /* In each line there are 4 tokens: name, available, cost and \n */
                switch(i%4){
                    case 0:
                    strcat(token, "\0");
                    strcpy(name, token);
                    break;
                    case 1:
                    tmp = token;
                    avail = atoi(tmp);
                    break;
                    case 2:
                    tmp = token;
                    cost = atoi(tmp);
                    break;
                }
                i++;
                token = strtok(NULL, ";");
            }
            printf("[client][%d][%d] Resource desired: %s %d %d \n", client_num, pid, name, avail, cost);
            // resourcesNumber++;

            required_resources[required_resources_length++] = name;
            // add_resource(name, avail, cost);
            fflush(stdout);
        }
    }else{
        fprintf(stderr, "[client][%d][%d] Error: Unable to open resource's file. %s\n", client_num, pid, strerror(errno));
    }

    // fflush(stdout);
    fclose(resources);
}


int main(int argc, char** argv){
    pid = getpid();
    ppid = getppid();
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", pid, ppid);

    // scrittura + lettura da file delle risorse: risorse da acquisire + loro quantità + budget
    // registrazione al banditore con messaggio che contiene pid + risorse che gli interessano
    // NB: dimensione del messaggio fissa
    if (argc >= 4 && strcmp(argv[1], "-m") == 0 && strcmp(argv[3], "-c") == 0){
        msqid = atoi(argv[2]);
        client_num = atoi(argv[4]);
        fprintf(stdout, "[client][%d][%d] Using message queue %d\n", client_num, pid, msqid);
    } else {
        fprintf(stderr, "[client][%d] Error: msqid (-m) argument not valid.\n", pid);
        return -1;
    }


    // char*

    load_resources();
    printf("[client] AAAAAAAAAAA\n");
    fflush(stdout);

    introduction* intr = (introduction*) malloc(sizeof(introduction));
    intr->pid = pid;
    // [TODO] Inserire la lista delle risorse richieste.
    /* Sends a message to the auctioneer containing the client pid and the required resources */
    msgsnd(msqid, intr, sizeof(introduction) - sizeof(long), 0777);


    // attesa della chiamata dal banditore
    // se banditore comunica acquisizione di risorsa --> scrivere su file di log : risorsa acquisita; quantità; prezzoComplessivo
    // fork --> agenti --> agenti exit --> client
    // lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato
    // exit()
    return 0;
}
