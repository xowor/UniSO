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


extern int errno;                   /* Externally declared (by kernel) */

typedef struct _client {
    pid_t client_pid;
    resource interested_resource[MAX_REQUIRED_RESOURCES];
} client;

int registered_clients[MAX_CLIENTS];        /* Array containing the PIDs of the registered clients*/
int registered_clients_count;               /* The number of the registered clients */


int master_msqid = 0;                       /* The id of the master message queue */
int pid_msqid[MAX_CLIENTS][2];              /* Array that associates the client PID to
                                             * his own message queue */
int semid;                                  /* The id of the the TAOs creation semaphore */
int opened_auctions = 0;                    /* The number of opened auctions */

resource_list* avail_resources;             /* The list containing all the available resources */
// int avail_resources->resources_count;           /* The number of all the available resources */



int canexit = 0;					        /* If set to 1 will (busy) wait for the auction system
                                             * termination */



int get_msqid_from_pid(int client_pid) {
    int j = 0;
    for (; j < MAX_CLIENTS; j++){
        if (pid_msqid[j][0] == client_pid){
            return pid_msqid[j][1];
        }
    }
}

/* Checks if the client with the given PID is registered */
int is_registered(int pid){
  int i = 0;
  for(; i < MAX_CLIENTS; i++){
    if(registered_clients[i] == pid)
      return 1;
  }
  return -1;
}

void deregister_client(int pid){
  int i = 0;
  for(; i < MAX_CLIENTS; i++){
    if(registered_clients[i] == pid){
        registered_clients[i] = 0;

        /* Kills the client */
        int p_status;
        kill(pid, SIGINT);
        waitpid(pid, &p_status, WUNTRACED);
    }
  }
}

/* Distributes a new message queue to each client. for each client first creates
 * the queue, then sends a message to the client containing the queue id.
 */
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


/* Loads the resources from file. */
void load_auct_resources() {
    avail_resources = create_resource_list();             //initializes resource list empty
    load_resources("../resource.txt", avail_resources);
}


/* Creates a tao structure for each resource. The shared memori will be NOT
 * created.
 */
void create_taos(){
	/* Creates TAO's array with empty tao + sem pool*/
    init_taos_array(avail_resources->resources_count);

	resource* tmp_resource = avail_resources->list;

	/* Creates the TAO structure */
	while(tmp_resource){
        create_tao(tmp_resource->name, tmp_resource->cost);
		tmp_resource = tmp_resource->next;
	}
}

/* Listens for the client status message. */
int listen_client_status(int client_msqid){
  client_status* msg = (client_status*) malloc(sizeof(client_status));

  if ( msgrcv(client_msqid, msg, sizeof(client_status) - sizeof(long), CLIENT_STATUS_MTYPE, 0) != -1 ) {
      if (msg->type == CLIENT_UNREGISTERED){
          deregister_client(client_msqid);
      }
      return 0;
  }
  return -1;

}

/* Notifies to each client registered to the given TAO that the TAO was created. */
void notify_tao_creation(tao* created_tao){
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

          /* Sends the message and waits for the client status */
          int client_msqid = get_msqid_from_pid(client_pid);
          msgsnd(client_msqid, msg, sizeof(auction_status) - sizeof(long), 0600);
          listen_client_status(client_msqid);

          free(msg);
        }
    }
}

/* Notifies to each client registered to the given TAO that the TAO was started. */
void notify_tao_start(tao* created_tao){
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

          /* Sends the message and waits for the client status */
          int client_msqid = get_msqid_from_pid(client_pid);
          msgsnd(client_msqid, msg, sizeof(auction_status) - sizeof(long), 0600);
          listen_client_status(client_msqid);

          free(msg);
        }
    }
}

/* Notifies to each client registered to the given TAO that the TAO was ended. */
void notify_tao_end(tao* created_tao){
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

          /* Sends the message and waits for the client status */
          int client_msqid = get_msqid_from_pid(client_pid);
          msgsnd(client_msqid, msg, sizeof(auction_status) - sizeof(long), 0600);
          listen_client_status(client_msqid);

          free(msg);
        }
    }
}

/* Notifies to each client registered to the given TAO the auction result,
 * specifying the name of the resource, and for each client the quantity and
 * the unit price won by it.
 */
void notify_auction_result(int client_pid, char* name, int quantity, int unit_bid){
    /* Allocates the simple_message message */
    auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
    msg->mtype = AUCTION_STATUS_MTYPE;
    msg->type = AUCTION_RESULT;
    strcpy(msg->resource, name);

    msg->quantity = quantity;
    msg->unit_bid = unit_bid;

    /* Sends the message and waits for the client status */
    int client_msqid = get_msqid_from_pid(client_pid);
    if(is_registered(client_pid) == 1){
        msgsnd(client_msqid, msg, sizeof(auction_status) - sizeof(long), 0600);
        listen_client_status(client_msqid);
    }

    free(msg);
}

int get_max_bid(tao* current_tao){
    int max_bid = 0;
    int max_bid_index = 0;
    int i = 0;
    for(; i < MAX_BIDS; i++){
        if(current_tao->bids[i].unit_bid > max_bid){
            max_bid = current_tao->bids[i].unit_bid;
            max_bid_index = i;
        }
    }
    return max_bid_index;
}



// int get_budget(int pid){
//   int i = 0;
//   for(; i < MAX_CLIENTS; i++){
//     if(registered_clients[i][0] == pid)
//       return registered_clients[i][1];
//   }
// }


/* Assigns the resources after the auction end, communicating the results to each
 * client.
 */
void assign_resources(tao* current_tao){
    printf("[\x1b[34mAuction\x1b[0m] %-16s || %17s || ", current_tao->name, "Assign resources");

    /* Builds a list containing the PIDs of all clients which partecipated to the auction */
    int i = 0;
    int losers[current_tao->interested_clients_count];
    for (; i < current_tao->interested_clients_count; i++){
        losers[i] = current_tao->interested_clients[i];
    }

    i = 0;
    for(; i < MAX_BIDS; i++){
        int max_bid_index = get_max_bid(current_tao);
        bid max_bid = current_tao->bids[max_bid_index];
        if (max_bid.client_pid > 1){
            resource* res = get_resource(current_tao->name, avail_resources);

            if(max_bid.quantity < res->availability){
                /* If the quantity requested in lesser than the quantity available */
                res->availability = res->availability - max_bid.quantity;
                printf("     %-5d : %4d      ||", max_bid.client_pid, max_bid.quantity);
                notify_auction_result(max_bid.client_pid, current_tao->name, max_bid.quantity, max_bid.unit_bid);
            }else{
                /* If the quantity requested in greter than the quantity available */
                /* assigns all the remaining resources*/
                printf("     %-5d : %4d      ||", max_bid.client_pid, res->availability);
                notify_auction_result(max_bid.client_pid, current_tao->name, res->availability, max_bid.unit_bid);
                res->availability = 0;
            }
            current_tao->bids[max_bid_index].unit_bid = 0;

            /* Sets to 0 the PID of the current client, so that the reult message
             * won't be sent twice
             */
            int j = 0;
            for (; j < current_tao->interested_clients_count; j++){
                if (losers[j] == max_bid.client_pid){
                    losers[j] = 0;
                }
            }
        }
    }

    printf("\n");

    /* Sends the result message to the losers clients */
    i = 0;
    for (; i < current_tao->interested_clients_count; i++){
        if (losers[i] > 1){
            notify_auction_result(losers[i], current_tao->name, 0, 0);
        }
    }



    // int k = 0;
    // for (; k < current_tao->interested_clients_count; k++){
    //     int client_pid = current_tao->interested_clients[k];
    //     int i = 0;
    //     int found_in_bids = 0;
    //     for(; i < MAX_BIDS; i++){
    //         if (current_tao->bids[i].client_pid == client_pid){
    //             found_in_bids = 1;
    //         }
    //     }
    //     if (!found_in_bids){
    //         notify_auction_result(current_tao->bids[i].client_pid, current_tao->name, 0, 0);
    //     }
    // }
}

/* Listens for the message from client with the client_pid and array_resources_wanted */
void listen_introductions(){

    introduction* intr = (introduction*) malloc(sizeof(introduction));
    registered_clients_count = 0;

    int i = 0;
    for (; i < MAX_CLIENTS; i++) {
        if ( msgrcv(pid_msqid[i][1], intr, sizeof(introduction) - sizeof(long), INTRODUCTION_MTYPE, 0) != -1 ){
            registered_clients[registered_clients_count] = intr->pid;
            registered_clients_count++;
            //printf("[auctioneer] Received auction partecipation request from pid %d\n", intr->pid);
            int j = 0;
            /* Registers the client to related tao */
            for (; j < intr->resources_length; j++){
                register_client_to_tao(intr->pid, intr->resources[j]); //add interested client to tao
            }
            pid_msqid[i][0] = intr->pid;
        }
    }
    free(intr);
}

/* Creates the tao process. */
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
    } else if ( tao_process_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
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

/* Starts the auction main loop */
void start_auction_system(){
    int tao_processes_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    tao* current_tao;
    int taos_total_count = avail_resources->resources_count;
    int created_taos = 0;
    int terminated_taos = 0;
    // int i = 1;

    while(terminated_taos < taos_total_count){
        if (created_taos < 3) {
            /* Creates the firs three TAOs without waiting for any tao process message */
            current_tao = get_tao(created_taos);
            init_tao(current_tao);
            notify_tao_creation(current_tao);
            create_tao_process(current_tao->id, current_tao->lifetime, tao_processes_msqid);
            created_taos++;
            printf("[\x1b[34mAuction\x1b[0m] %-16s ||  Starting three seconds timer (tao id: %d)\n", current_tao->name, current_tao->id);
        } else {
            simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
            /* Waits for a message from the tao process */
            if ( msgrcv(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {
                if ( strcmp(msg->msg, TAO_PROCESS_END_MSG) == 0 ){
                    /* The TAO must be ended */
                    int p_status;

                    /* Avoids zombie creation */
                    waitpid(msg->pid, &p_status, WUNTRACED);

                    current_tao = get_tao_by_id(msg->content.i);
                    notify_tao_end(current_tao);
                    assign_resources(current_tao);
                    terminated_taos++;
                    printf("[\x1b[34mAuction\x1b[0m] %-16s ||  Ended auction (tao id: %d)\n", current_tao->name, current_tao->id);

                    /* Deallocates shared memory and semaphore */
                    semctl(current_tao->sem_id, 0, IPC_RMID, 0);
                    shmctl(current_tao->shm_id, IPC_RMID, 0);

                    /* Creates a new TAO, if there is any left */
                    if (created_taos < taos_total_count){
                        current_tao = get_tao(created_taos);
                        init_tao(current_tao);
			            notify_tao_creation(current_tao);
                        create_tao_process(current_tao->id, current_tao->lifetime, tao_processes_msqid);
                        created_taos++;
                        printf("[\x1b[34mAuction\x1b[0m] %-16s ||  Starting three seconds timer (tao id: %d)\n", current_tao->name, current_tao->id);
                    }
                } else if ( strcmp(msg->msg, TAO_PROCESS_END_THREESEC) == 0 ){
                    /* The three seconds timer has ticked, so the TAO must start */
                    current_tao = get_tao(msg->content.i);
                    start_tao(current_tao);
                    notify_tao_start(current_tao);
                    printf("[\x1b[34mAuction\x1b[0m] %-16s ||  Started auction (tao id: %d)\n", current_tao->name, current_tao->id);
                }
            }
            free(msg);
        }
    }

	/* Removes the TAOS creation semaphore */
    semctl(semid, 0, IPC_RMID, 0);

    free(avail_resources);

    canexit == 1;
}


void kill_clients(){
    /* For each client interested in the TAO */
    int i = 0;
    // propagare sig int ai client
    for (; i < MAX_CLIENTS; i++){
      if(registered_clients[i] != 0){
        int p_status;
        kill(registered_clients[i], SIGINT);
        waitpid(registered_clients[i], &p_status, WUNTRACED);
      }
    }
}


/* Handles the sigint, dispatching it to the clients */
static void sigint_signal_handler () {
    kill_clients();

    wait(5);

    int i = 0;
    for(; i < MAX_CLIENTS; i++){
        if(registered_clients[i] != 0){
            int p_status;
            kill(registered_clients[i], SIGINT);
            waitpid(registered_clients[i], &p_status, WUNTRACED);
        }
    }


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
    // TO_SEE
    int i = 0;
    for(; i < MAX_CLIENTS; i++){
      registered_clients[i] = -1;
    }

    /**
     * Loads the message queue id from the passed argument (from MAIN)
     * -m : next position -> str_master_msquid
     */
    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        master_msqid = atoi(argv[2]);
        //fprintf(stdout, "[auctioneer] Using message queue %d\n", master_msqid);
    } else {
        fprintf(stderr, "[auctioneer] Error: master_msqid (-m) argument not valid.\n");
        return -1;
    }

    listen_sigint_signal();

    /* Sends each client his own message queue throught the master_msq */
    distribute_msqs();

	  /* Read resources from file */
    load_auct_resources();

    /* Create only the structure of all taos, without the client's list and relative bids */
    create_taos();

    /* subscription request from client to auctioneer*/
    listen_introductions();
    
    /* Removes the master_msq message queue */
    msgctl(master_msqid, IPC_RMID,0);


    /* Starts the auction system */
    start_auction_system();

    /* Busy wait */
    /* canexit == 0 --> tao just finished */
    while (canexit == 1) {}


    fprintf(stdout, "[auctioneer] \x1b[31mQuitting... \x1b[0m \n");
    fflush(stdout);
    _exit(EXIT_SUCCESS);
}
