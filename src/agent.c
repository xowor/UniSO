#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tao.c"
#include "config.h"
#include "semaphore.h"
#include "messages/tao_info_to_agent.h"
#include "messages/simple_message.h"

int msqid;
int unit_cost;
int quantity;
int shm_id;
int sem_id;
int basebid;
char* res;
int budget;
int current_bid = 0;				/* bid that starts like base_bid and grows at most like budget */
int tao_id;
pid_t pid;
tao* working_tao;

/**
* Support function to make_bid.
* substitutes bids[n] qualities with new ones
*/
void replace_bids(int n, bid* new_bid, tao* auction_tao){
    auction_tao->bids[n].client_pid    = new_bid->client_pid;
    auction_tao->bids[n].quantity     = new_bid->quantity;
    auction_tao->bids[n].unit_bid     = new_bid->unit_bid;
}



/**
* Adds bid from the agent.
* @param quantity How many elements the agent requires
* @param unit_offer How much the agent offer for each element
* @param auction_tao Tao connected to resource
* @return 1 if the operation end well, 0 otherwise
* Algoritmo:
*   * controlla l'offerta minima nel tao
* l'agente ha già un'offerta nel tao?
* 	se si --> controlla se l'offerta che vuole fare sia maggiore del minimo
* 		  se si --> la sovrascrive quella già presente
* 		  se no --> return
* 	se no --> controlla se l'offerta che vuole fare sia maggiore del minimo
* 		  se si --> scrive una nuova offerta a suo nome
* 			ci sono entry vuote?
* 				se si --> aggiunge la propria offerta nello spazio vuoto
* 				se no --> sovrascrive l'offerta di prezzo minore
* 		  se no --> return
*/
int make_bid(tao* auction_tao){
    // controlla l'offerta minima nel tao
    int i = 0;
    int min_bid     = 0;
    int index_min_bid = 0;
    int pid_min_bid = 0;

    /* Finds the min bid */
    for(; i < MAX_BIDS; i++){
        if(auction_tao->bids[i].unit_bid < min_bid){
            min_bid         = auction_tao->bids[i].unit_bid;
            index_min_bid   = i;
            pid_min_bid     = auction_tao->bids[i].client_pid;
        }
    }

    // nuovo bid
    bid* new_bid        = (bid*) malloc(sizeof(bid));
    new_bid->client_pid  = getppid();
    new_bid->quantity   = quantity;
    new_bid->unit_bid   = current_bid;


    // l'agente ha già un'offerta nel tao?
    int has_bid = 0, own_bid = NULL;
    for(i = 0; i < MAX_BIDS; i++){
        if( auction_tao->bids[i].client_pid == getppid() ){
            // so_log('b');
            has_bid = 1;
            own_bid = i;
        }
    }

    // controlla che l'offerta che vuole fare sia maggiore del minimo
    if(current_bid > min_bid){
        // ha già un'offerta nel tao --> sostituisce la "propria entry"
        if(has_bid){
            replace_bids(own_bid, new_bid, auction_tao);
            return 0;
            // non ha offerte nel tao
        }else{
            // aggiunge la propria offerta nello spazio vuoto
            // cerca entry vuota
            int empty_index = -1;
            for(i = 0; i < MAX_BIDS; i++){
                if(auction_tao->bids[i].client_pid == 0){
                    empty_index = i;
                }
            }
            if(empty_index >= 0){
                replace_bids(empty_index, new_bid, auction_tao);
                return 0;
            }else{
                // sovrascrive l'offerta di prezzo minore
                replace_bids(index_min_bid, new_bid, auction_tao);
                return 0;
            }
        }
    }else{
        return -2;
    }
    return -1;
}

void print_auction_status(tao* working_tao){
    printf("[\x1b[34mAuction\x1b[0m] %-16s || %8s || \x1b[33m%4d€\x1b[0m || ", working_tao->name, "New bid", best_bid());

    int i = 0;
    for (; i < 5; i++){
        if (working_tao->bids[i].client_pid == getppid()){
            printf("     \x1b[32m%-5d\x1b[0m : \x1b[33m%4d\x1b[0m\e[2m/%-4d\x1b[0m€||", working_tao->bids[i].client_pid, working_tao->bids[i].unit_bid, budget);
        } else {
            printf("     %-5d : \x1b[33m%4d€\x1b[0m     ||", working_tao->bids[i].client_pid, working_tao->bids[i].unit_bid);
        }
    }

    printf("\n");

    fflush(stdout);
}

/**
*  Check if agent in the argument is between the most bids.
*  @param pid_agent   Agent's pid who wants to make a bid
*  @param current_tao Tao where he needs to check the bids.
*  @return 1 if his bid is between best bid, 0 otherwise.
*/
int is_among_the_best_bids(){
    int i = 0;
    for(; i < MAX_BIDS; i++){
        if(working_tao->bids[i].client_pid == getppid()){
            return 1;
        }
    }
    return 0;
}


int worst_bid(){
    int min_bid = working_tao->bids[0].unit_bid;
    int i = 0;
    for(; i < MAX_BIDS; i++){
        if(working_tao->bids[i].unit_bid < min_bid){
            min_bid = working_tao->bids[i].unit_bid;
        }
    }
    return min_bid;
}


int best_bid(){
    int max_bid = 0;
    int i = 0;
    for(; i < MAX_BIDS; i++){
        if(working_tao->bids[i].unit_bid > max_bid){
            max_bid = working_tao->bids[i].unit_bid;
        }
    }
    return max_bid;
}

int get_availability_resources(){
  int total_price = quantity * unit_cost;
  while(total_price > budget){
    quantity--;
    total_price = quantity * unit_cost;
  }
  return quantity;
}

/**
* Adds to current bid a constant (if possible).
* @return 1 if it has done the sum, 0 otherwise.
* */
void increment_bid(){
    int new_bid = 0;
    if (quantity <= 0)
        return;
    // if((worst_bid() + BID_INCREMENT) > budget){
    //   return -1;
    // }
    //
    // current_bid = worst_bid() + BID_INCREMENT;
    // return 0;
    while( (new_bid = ((worst_bid() + BID_INCREMENT) * quantity)) > budget ){
        quantity--;
        if (quantity <= 0);

    }

    if (new_bid <= budget)
      current_bid = (worst_bid() + BID_INCREMENT);
    return;
}

/**
 * Makes a bid and checks the possibility.
 */
int make_action(int tao_id){
	sem_p(working_tao->sem_id, working_tao->id);
    // working_tao->dummy = 42;
    // if(is_among_the_best_bids() == 0){

    /* Tries to make a bid if   */
	if(worst_bid() >= current_bid){
		// if(current_bid < budget){

            /* If the bid can be increased (isn't greater than budget) */
            increment_bid();
			if(current_bid < budget){
				int val = make_bid(working_tao);

				if(val == 0){
                    print_auction_status(working_tao);
					// printf("[agent] [%d] The agent has made bid in the amount of %d for %s.\n", pid, current_bid, res);
				}else if(val == -2){
                    /* Make a new increased bid */
					make_action(tao_id);
				}else{      // -1
					// printf("[agent] [%d] There is already a bid or an error occurred in make_bid.\n", pid);
                    return -1;
				}
			}else{
                /* The increased bid would be greater than budget */
				// printf("[agent][%d][%d][%d][%s] The agent hasn't enough funds.\n", pid, current_bid, budget, working_tao->name);
                return -1;
			}
		// }else{
        //     // printf("[agent][%d] budget < current_bid.\n", pid);
        //     return -1;
        // }
	}else{
        // so_log_i('r', current_bid);
        // printf("[agent][%d] the bid is between the best.\n", pid);
    }
	sem_v(working_tao->sem_id, working_tao->id);
    return 0;
}

    /* da qui richiamo i metodi che fanno le offerte
     * Precondizione: l'agente ha già controllato se la sua offerta è tra le migliori --> non fa l'offerta con make bid
	 * Precondizione: l'agente ha già aumentato la unit_offer rispetto alla offerta precedente
	 * Precondizione: l'agente si può permettere (come budget) la unit_offer
	 * Fa l'offerta


*/

/**
* Listen to message from client about starting tao.
* The function stops when has received a message.
*/
void listen_tao_start(){
    simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
    if ( msgrcv(msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {
        tao_id = msg->content.i;
        // printf("[agent] Started making bids for resource %s.\tPid: %d\tPPid: %d\n", res, pid, getppid());
        // fflush(stdout);
        current_bid = 0;
		    while(make_action(tao_id) == 0){}
    }
    free(msg);
}

/**
 * Listen to message from client about tao informations.
 * The function stops when has received a message.
 */
void listen_tao_info(){
    tao_info_to_agent* msg = (tao_info_to_agent*) malloc(sizeof(tao_info_to_agent));
    if ( msgrcv(msqid, msg, sizeof(tao_info_to_agent) - sizeof(long), TAO_INFO_TO_AGENT_MTYPE, 0) != -1 ) {
    // [TODO] SEMAFORO PER LA LETTURA
        // so_log_is('m', getpid(), "started_agent");

        quantity = msg->availability;
        unit_cost = msg->cost;
        shm_id = msg->shmid;
        sem_id = msg->semid;
        basebid = msg->basebid;
        current_bid = basebid;
        budget = msg->budget;
        strcpy(res, msg->res);

        working_tao = (tao*) shmat(shm_id, NULL, 0);
    }
	free(msg);

}






int main(int argc, char** argv){
	pid = getpid();
    res = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);

	// printf("[agent] Started agent.\tPid: %d\tPPid: %d\n", pid, getppid());

	if (argc >= 2 && strcmp(argv[1], "-m") == 0 ){

        msqid = atoi(argv[2]);
        // strcpy(res, argv[4]);
        // so_log_s('y', "agent: TAO started");
        //fprintf(stdout, "[agent][%d] Using message queue %d\n", pid, msqid);
    } else {
        fprintf(stderr, "[agent][%d] Error: msqid (-m) argument not valid.\n", pid);
        return -1;
    }


    // associazione al tao shmget
    listen_tao_info();

    // listen tao start
    listen_tao_start();

}
