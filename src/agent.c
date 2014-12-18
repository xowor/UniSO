#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tao.c"
#include "config.h"
#include "semaphore.h"
#include "messages/tao_info_to_agent.h"

int msqid;
int aval;
int unit_cost;
int shmid;
int semid;
int basebid;
char res[MAX_RES_NAME_LENGTH];
int budget;
int current_bid;				/* bid that starts like base_bid and grows at most like budget */
tao* current_tao;
pid_t pid;

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
 * Listen to message from client about tao informations. 
 * The function stops when has received a message.
 */
void listen_tao_info(){
    // [TODO] SEMAFORO PER LA LETTURA
    tao_info_to_agent* msg = (tao_info_to_agent*) malloc(sizeof(tao_info_to_agent));
    if ( msgrcv(msqid, msg, sizeof(tao_info_to_agent) - sizeof(long), TAO_INFO_TO_AGENT_MTYPE, 0) != -1 ) {
        //so_log_is('m', getpid(), "started_agent");
        
        aval = msg->availability;
        unit_cost = msg->cost;
        shmid = msg->shmid;
        semid = msg->semid;
        basebid = msg->basebid;
        budget = msg->budget;
        strcpy(res, msg->res);
    }
	free(msg);
}

/**
 * Makes a bid and checks the possibility.
 */
void make_action(){
	sem_v(pid, current_tao->id);
	if(is_best_bid(getpid(), current_tao) == 0){
		if(current_bid < budget){
			if(increments_bid() == 1){
				//make_bid(int pid, int quantity, int unit_offer, tao* auction_tao)
				int val = make_bid(pid, aval, current_bid, current_tao);
				if(val == 1){
					printf("[agent] [%d] The agent has mad a bit in the amount of %d for %s.\n", pid, current_bid, res);
				}else if(val == -2){
					make_action();
				}else{
					printf("[agent] [%d] An error occurred in make_bid or there is already a bid.\n", pid);
				}
			}else{
				printf("[agent] [%d] The agent hasn't enough funds.\n", pid);
			}
		}
	}else{} // do anything, the bid is between the best
	sem_p(pid, current_tao->id);
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
	
	//printf("[agent] Started agent.\tPid: %d\tPPid: %d\n", pid, getppid());

	if (argc >= 2 && strcmp(argv[1], "-m") == 0 ){
        msqid = atoi(argv[2]);
        strcpy(res, argv[4]);
        //fprintf(stdout, "[agent][%d] Using message queue %d\n", pid, msqid);
    } else {
        fprintf(stderr, "[agent][%d] Error: msqid (-m) argument not valid.\n", pid);
        return -1;
    }
    
    // associazione al tao shmget
    listen_tao_info();
    
    current_bid = basebid;
    
    current_tao = get_tao_from_resource(res);
    
    // listen tao start
    
    while(1){
		make_action();
	}

}

