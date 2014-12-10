#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "tao.h"
#include "introduction.h"
#include "config.h"


#define MAX_RESOURCES 32            /* The maximum number of available resources */
#define MAX_OPEN_AUCTIONS 3         /* At the same time, there are at the height 3 tao */
#define N 20                        /* Size of "an array" */
#define MAX_RESOURCE 10				/* Max number of resources for each client */

extern int errno;                   /* Externally declared (by kernel) */

// mai in sleep

typedef struct _client {
    pid_t client_pid;
	resource interested_resource[MAX_RESOURCE];
} client;

int msqid = 0;                       /* The id of the message queue */
int opened_auctions = 0;             /* The number of opened auctions */

resource_list* avail_resources;      /* The list containing all the available resources */
int avail_resources_count;           /* The number of all the available resources */


// mai in sleep


/**
 * Reads from file all the available resources.
 */
int load_resources(){
    FILE* resources;
    char line[MAX_RES_NAME_LENGTH];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0;
    avail_resources_count = 0;

    avail_resources = (resource_list*) malloc(sizeof(resource_list));

    resources = fopen("../resource.txt", "r");
    if( resources != NULL ){
        /* Reads each line from file */
        while( fgets(line, MAX_RES_NAME_LENGTH + 32, resources) != NULL ){
            token = strtok(line, ";");
            i = 0;
            name = (char*) malloc(MAX_RES_NAME_LENGTH);
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
            avail_resources_count++;

            printf("[[NAME: %s]]", name);
			add_resource(avail_resources, name, avail, cost);
            fflush(stdout);
            fclose(resources);
            return 1;
        }
    }
    fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
    fclose(resources);
    return -1;    
}


/**
 * // SPOSTARLO IN TAO IN UN SECONDO MOMENTO
 * 	PRECONDIZIONE: SI HA UNA LISTA DI CLIENTI CON LE LORO RISORSE INTERESSATE!!!!!!
 *  definire i semafori ++ e -- dei semafori
 *  contattare i clienti con un messaggio
 *  definire la durata dell'asta
 *
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
 */
void create_taos(){
	/* creates tao's array with empty tao */
    init_taos(avail_resources_count);

	int i = 0;

	resource* tmp_resource = avail_resources->list;
	
	/* adds name's resource and common informations to each tao */
	while(tmp_resource->next){
		// printf("[AUCTIONEER CREAZIONE TAO] %s;%d;%d;\n", tmp_resource->name,tmp_resource->availability,tmp_resource->cost);
		// fflush(stdout);
        printf("\x1b[32m%s\x1b[0m\n",  get_tao(0)->name);
        create_tao(tmp_resource->name);
		tmp_resource = tmp_resource->next;
	}
    // printf("\x1b[32m%s\x1b[0m\n",  get_tao(0)->name);
}


int main(int argc, char** argv){
    printf("[auctioneer] Started auctioneer.\tPid: %d\tPPid: %d\n", getpid(), getppid());

    /**
     * Loads the message queue id from the passed argument.
     */
    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        msqid = atoi(argv[2]);
        //fprintf(stdout, "[auctioneer] Using message queue %d\n", msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

	/* Read resources from file */
    if(!load_resources())
		printf("Errore nel caricamento delle risorse.\n");
    /* create only the structure of all taos, without the client's list and relative bids */
    create_taos();
	//so_log();
	
    /**
     * Listen to all clients introduction.
     */
    introduction* intr = (introduction*) malloc(sizeof(introduction));
    int introd_count = 0;
	
	

    while ( (introd_count < MAX_CLIENTS) && (msgrcv(msqid, intr, sizeof(introduction) - sizeof(long), 0, 0) != -1)){
        introd_count++;
        int res_lenght = intr->resources_length;
        printf("[auctioneer] Received auction partecipation request from pid %d\n", intr->pid);
        int i = 0;
        for (; i < intr->resources_length; i++){
            sign_to_tao(intr->pid, intr->resources[i]);
            ////so_log();
            ////printf("[auctioneer] Client with pid %d requested partecipation for resource %s\n", intr->pid, intr->resources[i]);
        }
    }




    fprintf(stdout, "[auctioneer] \x1b[31mQuitting... \x1b[0m \n");
    fflush(stdout);
    
    /* because auctioneer is main's child */
    _exit(EXIT_SUCCESS);

}
