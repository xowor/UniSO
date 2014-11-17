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

int main(){
    printf("[auctioneer] Started auctioneer.Pid: %d\tPPid: %d\n", getpid(), getppid());
    fflush(stdout);
    
    /* read from file the resources and creates connected struct and tao */
    FILE* resources;
    resourcesList listaR = NULL;
    char buffer[50];
    //char name[10];
    char* name;
    char* tmp;
    char* token;
    int a = 0, c = 0, i = 0;
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
                        strcpy(name, token);
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
                //printf("#######  %s %d %d ######\n\n", name, a, c);
            }
            printf("#######  %s %d %d ######\n\n", name, a, c);
            fflush(stdout);
            //listaR = node_creation(name, a, c, NULL);
        }
    }else{
        fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
    }
    
    fflush(stdout);
    fclose(resources);
    
    
    /*
    int countAuction = 0;
    // read each resource's token 
    while( //ci sono elementi nella lista ){   

        if(countAuction < MAXTAO){
            //crea il tao corrispondente
            countAuction += 1;
        }else{
            // attesa che un'altra asta termini per farne partire un'altra
        }        
    }
    */
    exit(EXIT_SUCCESS);
    //return 0;
}
