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


int canexit = 0;					 /* ????? tmp ????? */


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


void load_auct_resources() {
    avail_resources = create_resource_list();
    load_resources("../resource.txt", avail_resources);
    // avail_resources = create_resource_list();
    // FILE* resources;
    // resources = fopen("../resource.txt", "r");
    // if( resources != NULL ){
    //     char line[64];
    //     while (fgets(line, 64, resources)){
    //         get_resource(line);
    //     }
    // }
}


// void load_auct_resources(){
//     FILE* resources;
//     char line[MAX_RES_NAME_LENGTH];
//     char* name;
//     char* tmp;
//     char* token;
//     int avail = 0, cost = 0, i = 0, resourcesNumber = 0;
//     // avail_resources->resources_count = 0;
//
//     avail_resources = create_resource_list();
//
//     resources = fopen("../resource.txt", "r");
//     if( resources != NULL ){
//         /* Reads each line from file */
//         while( fgets(line, MAX_RES_NAME_LENGTH + 32, resources) != NULL  && line ){
//             token = strtok(line, ";");
//             i = 0;
//             name = (char*) malloc(MAX_RES_NAME_LENGTH);
//             while( token ){
//                 /* In each line there are 4 tokens: name, available, cost and \n */
//                 switch(i%4){
//                     case 0:
//                         strcat(token, "\0");
//                         strcpy(name, token);
//                         break;
//                     case 1:
//                         tmp = token;
//                         avail = atoi(tmp);
//                         break;
//                     case 2:
//                         tmp = token;
//                         cost = atoi(tmp);
//                         break;
//                 }
//                 i++;
//                 token = strtok(NULL, ";");
//             }
//             printf("[auctioneer] Resource available: %s %d %d \n", name, avail, cost);
//             // avail_resources->resources_count++;
//
// 			add_resource(avail_resources, name, avail, cost);
//             fflush(stdout);
//             free(name);
//         }
//     } else {
//         fprintf(stderr, "[auctioneer] Error: Unable to open resource's file. %s\n", strerror(errno));
//         fclose(resources);
//     }
// }

void create_taos(){
	/* creates tao's array with empty tao */
    init_taos_array(avail_resources->resources_count);

	int i = 0;
	resource* tmp_resource = avail_resources->list;

	/* adds name's resource and common informations to each tao */
	while(tmp_resource){
        create_tao(tmp_resource->name);
		tmp_resource = tmp_resource->next;
	}
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
            //printf("[auctioneer] Received auction partecipation request from pid %d\n", intr->pid);
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


void create_tao_process(int lifetime, int tao_processes_msqid){
    char str_lifetime [32];
    char str_tao_processes_msqid[32];
    sprintf(str_lifetime, "%d", lifetime);
    sprintf(str_tao_processes_msqid, "%d", tao_processes_msqid);

    int tao_process_pid = fork();

    if ( tao_process_pid == -1 ){
        printf("[auctioneer] Error: Tao process not created.\n");
        fprintf(stderr, "\t%s\n", strerror(errno));
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if ( tao_process_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        // so_log_s('y', str_lifetime);
        // so_log_i('y', lifetime);
        char *argv[] = { "./tao_process", "-t", str_lifetime, "-m", str_tao_processes_msqid, NULL };
        /* Run the auctioneer process. */
        int auct_execve_err = execve("./tao_process", argv, envp);
        if (auct_execve_err == -1) {
            /* Cannot find the tao_process binary in the working directory */
            fprintf(stderr, "[Tao process] Error: cannot start tao_process process (Try running main from ./bin).\n");
            /* strerror nterprets the value of errnum, generating a string with a message that describes the error */
            fprintf(stderr, "\t%s\n", strerror(errno));
        }
    }
}


void start_auction_system(){
    int tao_processes_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    tao* current_tao;
    int tao_counter = 0;
    int i = 0;

    for(; i < ((avail_resources->resources_count*2) + 3); i++){
        if (tao_counter > 2) {
            simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
            if ( msgrcv(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {

                if (strcmp(msg->msg, TAO_PROCESS_END_MSG) == 0 ){
                        so_log_i('b', tao_counter);
                        so_log_i('b', avail_resources->resources_count);
                    if (tao_counter < avail_resources->resources_count){
                        current_tao = get_tao(i);
                        init_tao(current_tao);
                        tao_counter++;
                        create_tao_process(current_tao->lifetime, tao_processes_msqid);
                        // notify_tao_start(current_tao);
                    }
                } else if (strcmp(msg->msg, TAO_PROCESS_END_THREESEC) == 0 ){
                    so_log('c');
                }


            }
            free(msg);
        } else {
            current_tao = get_tao(i);
            init_tao(current_tao);
            create_tao_process(current_tao->lifetime, tao_processes_msqid);
            tao_counter++;
            // notify_tao_start(current_tao);
        }
    }

    canexit == 1;
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
        //fprintf(stdout, "[auctioneer] Using message queue %d\n", master_msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: master_msqid (-m) argument not valid.\n");
        return -1;
    }

    /* Sends each client his own message queue */
    distribute_msqs();

	/* Read resources from file */
    load_auct_resources();

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
