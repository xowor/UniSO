#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "resource.h"
#include "tao.h"
#include "introduction.h"

#define MAX_OPEN_AUCTIONS 3         /* At the same time, there are at the height 3 tao */
#define N 20                        /* Size of "an array" */

extern int errno;                   /* Externally declared (by kernel) */

// mai in sleep

/* registerd clients */
typedef struct client* clientList;
typedef struct _node{
    int client_pid;
    // risorse interessate (array)
    struct _node* next;
} client;


int msqid = 0;                       /* The id of the message queue*/
int opened_auctions = 0;             /* The number of opened auctions*/
resource_list* avail_resources;


// mai in sleep


void loadResources(){
    FILE* resources;
    char line[64];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0;

    avail_resources = (resource_list*) malloc(sizeof(resource_list));

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
            // resourcesNumber++;


            add_resource(avail_resources, name, avail, cost);
            fflush(stdout);
        }
    }else{
        fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
    }

    // fflush(stdout);
    fclose(resources);
}


// void create_tao(resource*){
//
// }

void create_taos(){
    char nameRes[N];
    // nameRes = NULL;
  /*  tao* taoElements = NULL;
    int id = 0;
    // read each resource's token from the struct
    int j;
    for(j = 0; j < resources_count; j++){
        // creates relative tao
        if(opened_auctions < MAX_OPEN_AUCTIONS){
            // recuperare il nome di una risorsa
            // strcpy(nameRes, resourceList->name);
            // eliminare la risorsa dalla lista delle risorse da assegnare
            // recuperare
            // allocare memoria condivisa
            // taoElements = node_creation(id, );
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
            opened_auctions += 1;
        }else{
            // attesa che un'altra asta termini per farne partire un'altra
        }
    }*/
}


int main(int argc, char** argv){
    printf("[auctioneer] Started auctioneer.\tPid: %d\tPPid: %d\n", getpid(), getppid());

    /* Loads the message queue id from the passed argument */
    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        msqid = atoi(argv[2]);
        fprintf(stdout, "[auctioneer] Using message queue %d\n", msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

    /* read from file the resources and creates connected struct and tao */
    loadResources();

    create_taos();

    introduction* intr = (introduction*) malloc(sizeof(introduction));
    while (msgrcv(msqid, intr, sizeof(introduction) - sizeof(long), 0, 0) != -1){
        int res_lenght = intr->resources_length;
        printf("[auctioneer] Received auction partecipation request from pid %d\n", intr->pid);
        int i = 0;
        for (; i < intr->resources_length; i++){
            printf("[auctioneer] Client with pid %d requested partecipation for resource %s\n", intr->pid, intr->resources[i]);
        }
    }


    fprintf(stdout, "[auctioneer] \x1b[31mQuitting... \x1b[0m \n");
    fflush(stdout);
    /* because auctioneer is main's child */
    _exit(EXIT_SUCCESS);

}
