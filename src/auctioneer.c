#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "resource.h"
#include "tao.h"

#define MAXTAO 3        /* At the same time, there are at the height 3 tao */
#define N 20

extern int errno;       /* Externally declared (by kernel) */

// mai in sleep

/* registerd clients */
typedef struct client* clientList;
typedef struct _node{
    int client_pid;
    // risorse interessate (array)
    struct _node* next;
}client;


int msqid;

// mai in sleep

int main(int argc, char** argv){
    printf("[auctioneer] Started auctioneer.\tPid: %d\tPPid: %d\n", getpid(), getppid());

    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        msqid = atoi(argv[2]);
        fprintf(stdout, "[auctioneer] Using message queue %d\n", msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

    /* read from file the resources and creates connected struct and tao */
    FILE* resources;
    resourcesList resourceList = NULL;
    resourcesList* nextNode = NULL;
    char line[64];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0;
    resources = fopen("../resource.txt", "r");
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
            printf("[auctioneer] Resource available: %s %d %d \n", name, avail, cost);
            resourcesNumber++;
            fflush(stdout);

            /* update the resource's struct */
            resourceList = node_creation(name, avail, cost, nextNode);
            nextNode = &resourceList;
        }
    }else{
        fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
    }

    fflush(stdout);
    fclose(resources);

    int countAuction = 0;
    int j = 0;
    char nameRes[N] = NULL;
    taoList* taoElements = NULL;
    int id = 0;
    /* read each resource's token from the struct */
    for(; j < resourcesNumber; j++){
        /* creates relative tao */
        if(countAuction < MAXTAO){
            // recuperare il nome di una risorsa
            strcpy(nameRes, resourceList->name);
            // eliminare la risorsa dalla lista delle risorse da assegnare
            // recuperare
            // allocare memoria condivisa
            taoElements = node_creation(id, );
            // informare clienti con un messaggio <id shm del tao, id semaforo, prezzo base d'asta>
            // TAO fa partire timer di 3 secondi
            // inizia l'asta
            // clienti fanno le offerte
            // durata dell'asta variabile - allo scadere banditore chiude asta
            // banditore legge contenuto tao
            // banditore assegna risorse secondo offerte migliori = invio di un messaggio ai clienti vincitori
            // ai clienti viene detratto il prezzo
            // viene aggiornata la lista delle risorse richieste dal cliente
            // deallocare memoria condivisa
            
            // ...
            countAuction += 1;
        }else{
            // attesa che un'altra asta termini per farne partire un'altra
        }
    }
    
    /* because auctioneer is main's son */
    _exit(EXIT_SUCCESS);

}
