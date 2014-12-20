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
int aval;
int unit_cost;
int shm_id;
int semid;
int basebid;
char* res;
int budget;
int current_bid;				/* bid that starts like base_bid and grows at most like budget */
int id_tao;
pid_t pid;
tao* working_tao;

/**
* Adds bid from the agent.
* @param pid Agent's pid who wants make this bid
* @param quantity How much elements the agent wants
* @param unit_offer How the agent offer for each element
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
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao){
    // controlla l'offerta minima nel tao
    int i = 0, index_min_bid = 0;
    int min_bid = auction_tao->bids[0].unit_offer;
    int pid_min_bid = auction_tao->bids[0].client_pid;

    for(; i < MAX_OFFERS; i++){
        if(auction_tao->bids[i].unit_offer < min_bid){
            min_bid = auction_tao->bids[i].unit_offer;
            index_min_bid = i;
            pid_min_bid = auction_tao->bids[i].client_pid;
        }
    }
    // nuovo bid
    bid* new_bid = (bid*) malloc(sizeof(bid));
    new_bid->client_pid = pid;
    new_bid->quantity = quantity;
    new_bid->unit_offer = unit_offer;

    // cerca entry vuota
    int empty_index = -1;
    for(i = 0; i < MAX_OFFERS; i++){
        if(auction_tao->bids[i].client_pid < 0){
            empty_index = i;
        }
    }

    // l'agente ha già un'offerta nel tao?
    int has_bid = 0, own_bid = NULL;
    for(i = 0; i < MAX_OFFERS; i++){
        if(auction_tao->bids[i].client_pid == pid){
            has_bid = 1;
            own_bid = i;
        }
    }

    // controlla che l'offerta che vuole fare sia maggiore del minimo
    if(unit_offer > min_bid){
        // ha già un'offerta nel tao --> sostituisce la "propria entry"
        if(has_bid){
            replace_bids(own_bid, new_bid, auction_tao);
            // non ha offerte nel tao
        }else{
            // aggiunge la propria offerta nello spazio vuoto
            if(empty_index >= 0){
                replace_bids(empty_index, new_bid, auction_tao);
                // sovrascrive l'offerta di prezzo minore
            }else{
                replace_bids(index_min_bid, new_bid, auction_tao);
            }
        }
    }else{
        return -2;
    }
    return -1;
}

/**
*  Check if agent in the argument is between the most bids.
*  @param pid_agent   Agent's pid who wants to make a bid
*  @param current_tao Tao where he needs to check the bids.
*  @return 1 if his bid is between best bid, 0 otherwise.
*/
int is_best_bid(){
    int i = 0;

    so_log_i('c', getpid());
    so_log_p('b', working_tao);
    for(; i < MAX_OFFERS; i++){
        if(working_tao->bids[i].client_pid == getpid()){
            return 1;
        }
    }
    return 0;
}

/**
 * Makes a bid and checks the possibility.
 */
void make_action(int id_tao){
    so_log('r');
    working_tao = (tao*) shmat(shm_id, NULL, 0);

	// sem_p(pid, current_tao->id);
	if(is_best_bid() == 0){
        so_log_i('m', pid);
		if(current_bid < budget){
			if(increments_bid() == 1){
                so_log('m');
				int val = make_bid(pid, aval, current_bid, working_tao);
				if(val == 1){
					printf("[agent] [%d] The agent has mad a bit in the amount of %d for %s.\n", pid, current_bid, res);
				}else if(val == -2){
					make_action(id_tao);
				}else{
					printf("[agent] [%d] An error occurred in make_bid or there is already a bid.\n", pid);
				}
			}else{
				printf("[agent] [%d] The agent hasn't enough funds.\n", pid);
			}
		}else{
            printf("[agent][%d] budget < current_bid.\n", pid);
        }
	}else{
        printf("[agent][%d] the bid is between the best.\n", pid);
    }
	// sem_v(pid, current_tao->id);
}

    /* da qui richiamo i metodi che fanno le offerte
     * Precondizione: l'agente ha già controllato se la sua offerta è tra le migliori --> non fa l'offerta con make bid
	 * Precondizione: l'agente ha già aumentato la unit_offer rispetto alla offerta precedente
	 * Precondizione: l'agente si può permettere (come budget) la unit_offer
	 * Fa l'offerta

    // attesa del segnale di avvio dell'asta
	// incrementa il semaforo
	// fa l'offerta
	// decrementa il semaforo
	// aspetta di nuovo il suo turno*/

/**
* Listen to message from client about starting tao.
* The function stops when has received a message.
*/
void listen_tao_start(){
    simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
    if ( msgrcv(msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {
        id_tao = msg->content.i;
        // [TODO] SEMAFORO PER LA LETTURA
		while(1){
			make_action(id_tao);
		}
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

        aval = msg->availability;
        unit_cost = msg->cost;
        shm_id = msg->shmid;
        semid = msg->semid;
        basebid = msg->basebid;
        budget = msg->budget;
        strcpy(res, msg->res);
    }
	free(msg);
}



/**
 * Adds to current bid a constant (if possible).
 * @return 1 if it has done the sum, 0 otherwise.
 * */
int increments_bid(){
	if((current_bid + BID_INCREMENT) > budget)
		return 0;
	else
		current_bid += BID_INCREMENT;
	return 1;
}


int main(int argc, char** argv){
	pid = getpid();
    res = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);

	printf("[agent] Started agent.\tPid: %d\tPPid: %d\n", pid, getppid());

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

    current_bid = basebid;

    // listen tao start
    listen_tao_start();

}
