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

    if (argc >= 2 && strcmp(argv[0], "-m") == 0){
        msqid = atoi(argv[1]);
        fprintf(stdout, "[auctioneer] Using message queue %d\n", msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

    /* read from file the resources and creates connected struct and tao */
    FILE* resources;
    resourcesList resourceList = NULL;
    resourcesList* nextNode = NULL;
    char buffer[50];
    char* name;
    char* tmp;
    char* token;
    int a = 0, c = 0, i = 0, resourcesNumber = 0;
    resources = fopen("../resource.txt", "r");
    if( resources != NULL ){
        /* read each line from file */
        while( fgets(buffer, 50, resources) != NULL ){
            /* read each token in the line */
            token = strtok(buffer, ";");
            i = 0;
            while( token ){
                switch(i%3){
                    case 0:
                        name = token;
                        //strcpy(name, token);
                        break;
                    case 1:
                        tmp = token;
                        a = atoi(tmp);
                        break;
                    case 2:
                        tmp = token;
                        c = atoi(tmp);
                        break;
                }
                i++;
                token = strtok(NULL, ";");
            }
            printf("#######  %s %d %d ######\n\n", name, a, c);
            resourcesNumber++;
            fflush(stdout);

            /* update the resource's struct */
            //resourceList = node_creation(name, a, c, nextNode);
            //nextNode = &resourceList;
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
        // read each resource's token
        

        if(countAuction < MAXTAO){
            // ...
            countAuction += 1;
        }else{
            // attesa che un'altra asta termini per farne partire un'altra
        }
    }
    
    exit(EXIT_SUCCESS);
    //return 0;
}
