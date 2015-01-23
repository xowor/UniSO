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
#include "messages/client_status.h"


#define N 20                        /* Size of "an array" */

extern int errno;                   /* Externally declared (by kernel) */

typedef struct _client {
  pid_t client_pid;
	resource interested_resource[MAX_REQUIRED_RESOURCES];
} client;

int client_list[MAX_CLIENTS];

int registered_clients;

int master_msqid = 0;                /* The id of the message queue */
int pid_msqid[MAX_CLIENTS][2];
// int msqsid[MAX_CLIENTS];             /* The message queues for each client */
int semid;                           /* The id of the the TAOs creation semaphore */
int opened_auctions = 0;             /* The number of opened auctions */

resource_list* avail_resources;      /* The list containing all the available resources */
// int avail_resources->resources_count;           /* The number of all the available resources */



int canexit = 0;					 /* ????? tmp ????? */

int is_registered(int pid){
  int i = 0;
  for(; i < MAX_CLIENTS; i++){
    if(client_list[i] == pid)
      return 1;
  }
  return -1;
}

void client_unregistration(int pid){
  int i = 0;
  for(; i < MAX_CLIENTS; i++){
    if(client_list[i] == pid)
      client_list[i] = 0;
  }
}

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


int listen_client_status(int client_msqid){
  client_status* msg = (client_status*) malloc(sizeof(client_status));
  int j = 0;
  if ( msgrcv(client_msqid, msg, sizeof(client_status) - sizeof(long), CLIENT_STATUS_MTYPE, 0) != -1 ) {
      if (msg->type == CLIENT_UNREGISTERED)
          client_unregistration(msg->pid);
    return 0;
  }
  return -1;

}

void notify_tao_creation(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        if(is_registered(client_pid) == 1){
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
                  listen_client_status(pid_msqid[j][1]);
              }
          }
          free(msg);
        }
    }
}

void notify_tao_start(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        if(is_registered(client_pid) == 1){
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
                    //   so_log('r');
                  listen_client_status(pid_msqid[j][1]);
              }
          }
          free(msg);
        }
    }
}

void notify_tao_end(tao* created_tao){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < created_tao->interested_clients_count; i++){
        int client_pid = created_tao->interested_clients[i];

        if(is_registered(client_pid) == 1){
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
                  listen_client_status(pid_msqid[j][1]);
              }
          }
          free(msg);
        }
    }
}

void notify_auction_result(int client_pid, char* name, int quantity, int unit_bid){
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
          if(is_registered(client_pid) == 1){
            msgsnd(pid_msqid[j][1], msg, sizeof(auction_status) - sizeof(long), 0600);
            listen_client_status(pid_msqid[j][1]);
          }
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



// int get_budget(int pid){
//   int i = 0;
//   for(; i < MAX_CLIENTS; i++){
//     if(client_list[i][0] == pid)
//       return client_list[i][1];
//   }
// }

void assign_resources(tao* current_tao){
        so_log('b');
    printf("[\x1b[34mAuction\x1b[0m] %-16s || %20s ||          ", current_tao->name, "Assign resources");

    int k = 0;
    int i = 0;
    for(; i < MAX_BIDS; i++){
        int max_bid_n = get_max_bid(current_tao);
        bid max_bid = current_tao->bids[max_bid_n];
        resource* res = get_resource(current_tao->name, avail_resources);
        // so_log_i('r', res->availability);
        // if( res->availability != 0 ){
        // so_log_i('y', max_bid.client_pid);
        // int budget = get_budget(max_bid.client_pid);
        // so_log_i('y', budget);
        if(max_bid.quantity < res->availability){
            // int amount = get_availability_resources(max_bid.quantity, max_bid.unit_bid, budget);
            // res->availability = res->availability - amount;
            // printf("     %-5d : %4d      ||", max_bid.client_pid, amount);
            // notify_auction_result(max_bid.client_pid, current_tao->name, amount, max_bid.unit_bid);
            res->availability = res->availability - max_bid.quantity;
            printf("     %-5d : %4d      ||", max_bid.client_pid, max_bid.quantity);
            notify_auction_result(max_bid.client_pid, current_tao->name, max_bid.quantity, max_bid.unit_bid);
        }else{
            // int amount = get_availability_resources(res->availability, max_bid.unit_bid, budget);
            // printf("     %-5d : %4d      ||", max_bid.client_pid, amount);
            // notify_auction_result(max_bid.client_pid, current_tao->name, amount, max_bid.unit_bid);
            res->availability = res->availability - res->availability;
            printf("     %-5d : %4d      ||", max_bid.client_pid, res->availability);
            notify_auction_result(max_bid.client_pid, current_tao->name, res->availability, max_bid.unit_bid);
        }
        current_tao->bids[max_bid_n].unit_bid = 0;
    }

    printf("\n");

    for (; k < current_tao->interested_clients_count; k++){
        int client_pid = current_tao->interested_clients[k];
        int i = 0;
        int found_in_bids = 0;
        for(; i < MAX_BIDS; i++){
            if (current_tao->bids[i].client_pid == client_pid){
                found_in_bids = 1;
            }
        }
        if (!found_in_bids){
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
            client_list[registered_clients] = intr->pid;
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

void start_auction_system(){
    int tao_processes_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    tao* current_tao;
    int tao_counter = 0;
    int i = 1;
    int terminated_taos = 0;

    while(terminated_taos < avail_resources->resources_count){
        if (tao_counter > 2) {
            simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
            if ( msgrcv(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {

                if ( strcmp(msg->msg, TAO_PROCESS_END_MSG) == 0 ){
                    current_tao = get_tao_by_id(msg->content.i);
                    // so_log_i('b', current_tao->dummy);
                    notify_tao_end(current_tao);
                    assign_resources(current_tao);
                    printf("[\x1b[34mAuction\x1b[0m] %-16s ||  \x1b[31mEnded\x1b[0m auction at %u (tao id: %d)\n", current_tao->name, (unsigned)time(NULL), current_tao->id);
                    semctl(current_tao->sem_id, 0, IPC_RMID, 0);
                    shmctl(current_tao->shm_id, IPC_RMID, 0);
                    // deallocare semafori
                    terminated_taos++;
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
                    printf("[\x1b[34mAuction\x1b[0m] %-16s ||  \x1b[32mStarted\x1b[0m auction at %u (tao id: %d)\n", current_tao->name, (unsigned)time(NULL), current_tao->id);
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
        semctl(sem_id, i, IPC_RMID, 0);
    }

    /* Removes all the TAOS shared memory  */
    for (i = 0; i < taos_count; i++){
        shmctl(taos[i]->shm_id, IPC_RMID, 0);
    }
}

void gc(){
    free(avail_resources);
}

void notify_clients_unregistration(){
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* For each client interested in the TAO */
    int i = 0;
    for (; i < registered_clients; i++){
      int client_pid = client_list[i];

      /* Allocates the simple_message message */
      auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
      msg->mtype = AUCTION_STATUS_MTYPE;
      msg->type = UNREGISTRATION;

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


// la deregistrazione di un cliente -> eliminazione da  lista -> non più considerato nelle restanti funzioni
static void sigint_signal_handler () {
	notify_clients_unregistration();

  wait(5);

  int i = 0;
  for(; i < MAX_CLIENTS; i++){
    if(client_list[i] != 0)
      kill(client_list[i], SIGKILL);
  }

  // si uccidono
  // l'auctioneer pulisce la lista dei clienti e li uccide
  // pulisce la lista di clienti


	fprintf(stdout, "[auctioneer] \x1b[31mRemoving all the IPC structures... \x1b[0m \n");
    ipc_gc();

    fprintf(stdout, "[auctioneer] \x1b[31mCleaning heap... \x1b[0m \n");
    gc();

    fprintf(stdout, "[auctioneer] \x1b[31mQuitting... \x1b[0m \n");
    fflush(stdout);
	 _exit(EXIT_SUCCESS);
}

void listen_sigint_signal(){
	if (signal(SIGINT, sigint_signal_handler)== SIG_ERR)
		perror("signal (SIG_ERR) error");
}

int main(int argc, char** argv){
    printf("[auctioneer] Started auctioneer.\tPid: %d\tPPid: %d\n", getpid(), getppid());
    //
    // int i = 0;
    // for(; i < MAX_CLIENTS; i++){
    //   client_list[i][0] = 0;
    //   client_list[i][1] = -1;
    // }

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

	  listen_sigint_signal();

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
