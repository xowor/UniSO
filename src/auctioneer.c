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
#include "messages/auction_result.h"
#include "messages/auction_status.h"


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
    /*
    // avail_resources = create_resource_list();
    // FILE* resources;
    // resources = fopen("../resource.txt", "r");
    // if( resources != NULL ){
    //     char line[64];
    //     while (fgets(line, 64, resources)){
    //         get_resource_from_line(line);
    //     }
    // }
    * */
}

/*
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
//         // Reads each line from file
//         while( fgets(line, MAX_RES_NAME_LENGTH + 32, resources) != NULL  && line ){
//             token = strtok(line, ";");
//             i = 0;
//             name = (char*) malloc(MAX_RES_NAME_LENGTH);
//             while( token ){
//                 // In each line there are 4 tokens: name, available, cost and \n
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
*/
void create_taos(){
	/* creates tao's array with empty tao */
    init_taos_array(avail_resources->resources_count);

	int i = 0;
	resource* tmp_resource = avail_resources->list;

	/* adds name's resource and common informations to each tao */
	while(tmp_resource){
        create_tao(tmp_resource->name, tmp_resource->cost);
		tmp_resource = tmp_resource->next;
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
        auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
        msg->mtype = AUCTION_STATUS_MTYPE;
        msg->type = AUCTION_CREATED;
        strcpy(msg->resource, created_tao->name);

        msg->shm_id = created_tao->shm_id;
        msg->sem_id = created_tao->sem_id;
        msg->base_bid = created_tao->base_bid;

        /* Gets the message queue id of the client */
        int j = 0;
        for (; j < MAX_CLIENTS; j++){
            if (pid_msqid[j][0] == client_pid){
                // so_log('r');
                msgsnd(pid_msqid[j][1], msg, sizeof(auction_status) - sizeof(long), 0600);
            }
        }
        free(msg);
    }
}


void notify_tao_start(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        /* Allocates the simple_message message */
        auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
        msg->mtype = AUCTION_STATUS_MTYPE;
        msg->type = AUCTION_STARTED;

        strcpy(msg->resource, created_tao->name);

        /* Gets the message queue id of the client */
        int j = 0;
        for (; j < MAX_CLIENTS; j++){
            if (pid_msqid[j][0] == client_pid){
                msgsnd(pid_msqid[j][1], msg, sizeof(auction_status) - sizeof(long), 0600);
                    // so_log('m');
            }
        }
        free(msg);
    }
}


void notify_tao_end(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        /* Allocates the simple_message message */
        auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
        msg->mtype = AUCTION_STATUS_MTYPE;
        msg->type = AUCTION_ENDED;

        strcpy(msg->resource, created_tao->name);

        /* Gets the message queue id of the client */
        int j = 0;
        for (; j < MAX_CLIENTS; j++){
            if (pid_msqid[j][0] == client_pid){
                msgsnd(pid_msqid[j][1], msg, sizeof(auction_status) - sizeof(long), 0600);
            }
        }
        free(msg);
    }
}


notify_auction_result(int client_pid, char* name, int quantity, int unit_bid){
    /* Allocates the simple_message message */
    auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
    msg->mtype = AUCTION_STATUS_MTYPE;
    msg->type = AUCTION_RESULT;
    strcpy(msg->resource, name);

    msg->quantity = quantity;
    msg->unit_bid = unit_bid;

    /* Gets the message queue id of the client */
    int j = 0;
    for (; j < MAX_CLIENTS; j++){
        if (pid_msqid[j][0] == client_pid){
            msgsnd(pid_msqid[j][1], msg, sizeof(auction_status) - sizeof(long), 0600);
        }
    }
    free(msg);
}

int get_max_bid(tao* current_tao){
    int max_bid = 0;
    int max_index = 0;
    int i = 0;
    for(; i < MAX_BIDS; i++){
        if(current_tao->bids[i].unit_bid > max_bid){
            max_bid = current_tao->bids[i].unit_bid;
            max_index = i;
        }
    }
    return max_index;
}

void assign_resources(tao* current_tao){
    // get_max_bid che ritorna il bid massimo, gli si passa current_tao->bids[]
    // per quel bid invia il messaggio di vincita/perdita
    // annulla i campi di quel bid



    int k = 0;

    int i = 0;
    for(; i < MAX_BIDS; i++){
        int max_bid_n = get_max_bid(current_tao);
        bid max_bid = current_tao->bids[max_bid_n];
        resource* res = get_resource(current_tao->name, avail_resources);
        // so_log_i('r', res->availability);
        // if( res->availability != 0 ){
        if(max_bid.quantity < res->availability){
            res->availability = res->availability - max_bid.quantity;
            // so_log_i('g', current_tao->bids[max_bid_n].client_pid);
            notify_auction_result(max_bid.client_pid, current_tao->name, max_bid.quantity, max_bid.unit_bid);
        }else{
            // so_log_i('m', current_tao->bids[max_bid_n].client_pid);
            notify_auction_result(max_bid.client_pid, current_tao->name, res->availability, max_bid.unit_bid);
        }
        current_tao->bids[max_bid_n].unit_bid = 0;
    }
        // }else{
        //     so_log_s('r', current_tao->name);
        //     notify_auction_result(current_tao->bids[i].client_pid, current_tao->name, 0, 0);
        // }
        // int max_bid = 0;
        // int max_bid_pid = 0;
        // int j = 0;
        // for (; j < MAX_BIDS; j++){
        //     resource* res = get_resource(current_tao->name, avail_resources);
        //     if( res->availability != 0 ){
        //         if (max_bid < current_tao->bids[j].unit_bid){
        //             max_bid = current_tao->bids[j].unit_bid;
        //             if(current_tao->bids[j].quantity < res->availability){
        //                 res->availability = res->availability - current_tao->bids[j].quantity;
        //                 so_log_s('g', current_tao->name);
        //                 notify_auction_result(current_tao->bids[j].client_pid, current_tao->name, current_tao->bids[j].quantity, current_tao->bids[j].unit_bid);
        //             }else{
        //                 so_log_s('g', current_tao->name);
        //                 notify_auction_result(current_tao->bids[j].client_pid, current_tao->name, res->availability, current_tao->bids[j].unit_bid);
        //             }
        //             max_bid = 0;
        //         }
        //     }else{
        //         // i = MAX_BIDS;
        //         so_log_s('r', current_tao->name);
        //         notify_auction_result(current_tao->bids[i].client_pid, current_tao->name, 0, 0);
        //     }
        // }
        // notificare i perdenti


    for (; k < current_tao->interested_clients_count; k++){
        int client_pid = current_tao->interested_clients[k];
        int i = 0;
        int found_in_bids = 0;
        for(; i < MAX_BIDS; i++){
            // so_log_i('y', client_pid);
            // so_log_i('y', current_tao->bids[i].client_pid);
            if (current_tao->bids[i].client_pid == client_pid){
                found_in_bids = 1;
            }
        }
        if (!found_in_bids){
            // so_log('r');
            notify_auction_result(current_tao->bids[i].client_pid, current_tao->name, 0, 0);
        }
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


void create_tao_process(int id_tao, int lifetime, int tao_processes_msqid){
    char str_lifetime [32];
    char str_tao_processes_msqid[32];
    char str_id_tao[32];
    sprintf(str_lifetime, "%d", lifetime);
    sprintf(str_tao_processes_msqid, "%d", tao_processes_msqid);
    sprintf(str_id_tao, "%d", id_tao);

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
        char *argv[] = { "./tao_process", "-t", str_lifetime, "-m", str_tao_processes_msqid, "-i",  str_id_tao, NULL };
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

/**
crea il tao processo
notifica che è stato creato
aspetta tre secondi
notifica end tre secondi
tao processo lifetime
notifica end lifetime
 */
void start_auction_system(){
    int tao_processes_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    tao* current_tao;
    int tao_counter = 0;
    int i = 1;

    int terminated_taos = 0;

	//so_log_i('r', avail_resources->resources_count);
    while(terminated_taos < avail_resources->resources_count){
        // SISTEMARE SEMAFORI
		//so_log_i('r', tao_counter);
        if (tao_counter > 2) {
            simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
            if ( msgrcv(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {

                if ( strcmp(msg->msg, TAO_PROCESS_END_MSG) == 0 ){
                    current_tao = get_tao_by_id(msg->content.i);
                    // so_log_i('b', current_tao->dummy);
                    notify_tao_end(current_tao);
                    assign_resources(current_tao);
                    printf("[auctioneer] Ended auction for resource %s (tao id: %d)\n", current_tao->name, current_tao->id);
                    shmctl(current_tao->shm_id, IPC_RMID, 0);
                    // so_log_i('r', terminated_taos);
                    terminated_taos++;
                    // assegna le risorse e annuncia il vincitore
                    // deallocazione tao + semafori
					/* If there are other resources tao to be created*/
                    if (tao_counter < avail_resources->resources_count){
                        current_tao = get_tao(tao_counter);
                        init_tao(current_tao);
						notify_tao_creation(current_tao);
                        tao_counter++;
                        create_tao_process(current_tao->id, current_tao->lifetime, tao_processes_msqid);
                    }
                } else if ( strcmp(msg->msg, TAO_PROCESS_END_THREESEC) == 0 ){
					current_tao = get_tao(msg->content.i);
                    start_tao(current_tao);
					notify_tao_start(current_tao);
                    // so_log_i('m', i++);
                    printf("[auctioneer] Started auction for resource %s (tao id: %d)\n", current_tao->name, current_tao->id);
                }

            }
            free(msg);
        } else {
            current_tao = get_tao(tao_counter);
            init_tao(current_tao);
            notify_tao_creation(current_tao);
            create_tao_process(current_tao->id, current_tao->lifetime, tao_processes_msqid);
            tao_counter++;
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
