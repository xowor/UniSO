#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include "resource.h"
#include "config.h"
#include "tao.h"
#include "semaphore.h"
#include "messages/introduction.h"
#include "messages/simple_message.h"
#include "messages/tao_opening.h"


#define N 20                        /* Size of "an array" */

extern int errno;                   /* Externally declared (by kernel) */

// mai in sleep

typedef struct _client {
    pid_t client_pid;
	resource interested_resource[MAX_REQUIRED_RESOURCES];
} client;

int master_msqid = 0;                /* The id of the message queue */
int pid_msqid[MAX_CLIENTS][2];
// int msqsid[MAX_CLIENTS];             /* The message queues for each client */
int semid;                           /* The id of the the TAOs creation semaphore */
int opened_auctions = 0;             /* The number of opened auctions */

resource_list* avail_resources;      /* The list containing all the available resources */
// int avail_resources->resources_count;           /* The number of all the available resources */

int registered_clients;
int canexit = 0;					 /* ?????????? */


void distribute_msqs(){
    int i = 0;
    for (; i < MAX_CLIENTS; i++){
        int msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, MSG_QUEUE_MSG);
        msg->content.i = msqid;

        msgsnd(master_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);
        free(msg);

        pid_msqid[i][1] = msqid;
    }
}

void load_resources(){
    FILE* resources;
    char line[MAX_RES_NAME_LENGTH];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0;
    // avail_resources->resources_count = 0;

    avail_resources = create_resource_list();

    resources = fopen("../resource.txt", "r");
    if( resources != NULL ){
        /* Reads each line from file */
        while( fgets(line, MAX_RES_NAME_LENGTH + 32, resources) != NULL  && line ){
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
            // avail_resources->resources_count++;

			add_resource(avail_resources, name, avail, cost);
            fflush(stdout);
            free(name);
        }
    } else {
        fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
        fclose(resources);
    }
}



void create_taos(){
	/* creates tao's array with empty tao */
    init_taos(avail_resources->resources_count);

	int i = 0;
	resource* tmp_resource = avail_resources->list;

	/* adds name's resource and common informations to each tao */
	while(tmp_resource){
        create_tao(tmp_resource->name);
		tmp_resource = tmp_resource->next;
	}
}



void alarm_handler() {
	// viene inviato un segnale ai clienti
	// gli agenti a turno iniziano ad inviare le offerte
	// sigalarm per la durata dell'asta + sigint per avvertire i clienti
	// tao che muore decrementa semaforo
    canexit = 0;
}



// alla ricezione del messaggio, il cliente deve solo creare l'agente
void notify_tao_start(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        /* Allocates the simple_message message */
        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, AUCTION_START_MSG);
        strcpy(msg->content.s, created_tao->name);

        /* Gets the message queue id of the client */
        int j = 0;
        for (; j < MAX_CLIENTS; j++){
            if (pid_msqid[j][0] == client_pid){
                msgsnd(pid_msqid[j][1], msg, sizeof(simple_message) - sizeof(long), 0600);
            }
        }
        free(msg);
    }
}



// alla ricezione del messaggio, il cliente deve solo creare l'agente
void notify_tao_creation(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        /* Allocates the simple_message message */
        tao_opening* msg = (tao_opening*) malloc(sizeof(tao_opening));
        msg->mtype = TAO_OPENING_MTYPE;
        strcpy(msg->resource, created_tao->name);
        msg->shmid = created_tao->shm_id;
        msg->semid = created_tao->sem_id;
        msg->base_bid = created_tao->base_bid;

        /* Gets the message queue id of the client */
        int j = 0;
        for (; j < MAX_CLIENTS; j++){
            if (pid_msqid[j][0] == client_pid){
                msgsnd(pid_msqid[j][1], msg, sizeof(tao_opening) - sizeof(long), 0600);
            }
        }
        free(msg);
    }
}


void listen_introductions(){
    // [TODO] SEMAFORO PER LA LETTURA
    introduction* intr = (introduction*) malloc(sizeof(introduction));
    registered_clients = 0;

    int i = 0;
    for (; i < MAX_CLIENTS; i++) {
        if ( msgrcv(pid_msqid[i][1], intr, sizeof(introduction) - sizeof(long), INTRODUCTION_MTYPE, 0) != -1 ){
            registered_clients++;
            printf("[auctioneer] Received auction partecipation request from pid %d\n", intr->pid);
            int j = 0;
            /* associates the pid's client to related tao, where there is the resource interested to client */
            for (; j < intr->resources_length; j++){
                sign_to_tao(intr->pid, intr->resources[j]);
            }
            pid_msqid[i][0] = intr->pid;
        }
    }
    free(intr);
}



void start_auction_system(){
    int i = 0;
    tao* current_tao;
	for(; i < avail_resources->resources_count; i++){
		/* "pauses" the tao creation until there are 3 tao opened */
		// GIUSTO while(semctl(tao_access_semid, 0, GETVAL, 0) > 2){	}

		/* gets one tao from the array */
		current_tao = get_tao(i);

		/* Associates each tao to an shm */
		start_tao(current_tao);

        /* Increments semaphore */
        // DECREMENTARLO ALLA DEALLOCAZIONE DEL TAO!!!!!!!!!!!!!!!!!!!!!!!!!
        sem_v(tao_access_semid, 0);

		/* says to client starting tao */
        notify_tao_creation(current_tao);
        notify_tao_start(current_tao);

    	/* timer of 3 seconds before the start of auction */
    	if(signal(SIGALRM, alarm_handler) == SIG_ERR)
    		printf("Error in alarm signal");
        alarm(3);
	}


}



void ipc_gc(){
    int i;

    /* Removes the queue message */
    msgctl(master_msqid, IPC_RMID,0);

	/* Removes the TAOS creation semaphor */
	// if(semctl(sem_id, 0, GETVAL, 0) == 0) {
	semctl(semid, 0, IPC_RMID, 0);

    /* Removes all the TAOS access semaphor */
    for (i = 1; i <= taos_count; i++){              /* Sem. 0 is the TAO creation one */
        semctl(tao_access_semid, i, IPC_RMID, 0);
    }

    /* Removes all the TAOS shared memory  */
    for (i = 0; i < taos_count; i++){
        shmctl(taos[i]->shm_id, IPC_RMID, 0);
    }
}



void gc(){
    free(avail_resources);
}



int main(int argc, char** argv){
    printf("[auctioneer] Started auctioneer.\tPid: %d\tPPid: %d\n", getpid(), getppid());

    /**
     * Loads the message queue id from the passed argument.
     */
    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        master_msqid = atoi(argv[2]);
        fprintf(stdout, "[auctioneer] Using message queue %d\n", master_msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: master_msqid (-m) argument not valid.\n");
        return -1;
    }

    /* Sends each client his own message queue */
    distribute_msqs();

	/* Read resources from file */
    load_resources();

    /* Create only the structure of all taos, without the client's list and relative bids */
    create_taos();

    listen_introductions();

    /* Start max 3 tao at a time */
    start_auction_system();

    while (canexit == 1) {}

    fprintf(stdout, "[auctioneer] \x1b[31mRemoving all the IPC structures... \x1b[0m \n");
    ipc_gc();

    fprintf(stdout, "[auctioneer] \x1b[31mCleaning heap... \x1b[0m \n");
    gc();

    fprintf(stdout, "[auctioneer] \x1b[31mQuitting... \x1b[0m \n");
    fflush(stdout);
    /* because auctioneer is main's child */
    _exit(EXIT_SUCCESS);
}
