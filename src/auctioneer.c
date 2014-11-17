#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "resource.h"

#define MAXTAO 3        /* At the same time, there are at the height 3 tao */
#define N 20

extern int errno;       /* Externally declared (by kernel) */

// mai in sleep
// un tao per risorsa
// area di memoria condivisa
// max 5 offerte per tao
typedef struct{
    int client_pid;
    int availability;
    int unitOffer;
}tao;

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
            // resourceList = node_creation(name, avail, cost, nextNode);
            // nextNode = &resourceList;
        }
    }else{
        fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
    }

    fflush(stdout);
    fclose(resources);

    int countAuction = 0;
    int j = 0;
    /* read each resource's token from the struct */
    for(; j < resourcesNumber; j++){
        /* creates relative tao */
        if(countAuction < MAXTAO){
            // ...
            countAuction += 1;
        }else{
            // attesa che un'altra asta termini per farne partire un'altra
        }
    }

    exit(EXIT_SUCCESS);

}
