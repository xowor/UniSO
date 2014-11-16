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
    resources = fopen("../resource.txt", "r");
    resourcesList listaR = NULL;
    char buffer[50];
    char* tmp;
    char* name;
    int a, c, i = 0;
    char* line;
    // lettura da file http://forum.ubuntu-it.org/viewtopic.php?t=313100
    if( resources != NULL ){ 
        while(!feof(resources)){
            line = fgets(buffer, 50, resources);
            // legge la linea, finchè ce n'è
            if( line != NULL ){
                tmp = strtok(line, ";");
                // legge i token
                if( tmp != NULL ){
                    switch(i%3){
                        case 0:
                            name = tmp;
                            break;
                        case 1:
                            a = *tmp;
                            //printf("a: %d", a);
                            break;
                        case 2:
                            c = *tmp;
                            //printf("c: %d", c);
                            break;
                    }
                    //listaR = node_creation(name, a, c, NULL);
                    printf("i=%d:\t%s\t%d\t%d\n", i, name, a, c);
                    tmp = strtok(NULL, ";");
                    i++;
                }
            }else{
                fprintf(stderr, "[auctioneer] Error: fgets. %s\n", strerror(errno));
            }
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
