#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include "so_log.h"
#include "config.h"

/**
 * Start (crates the shared memory area) the TAO with the given name.
            // inizia l'asta
            // clienti fanno le offerte
            // allo scadere banditore chiude asta
            // banditore legge contenuto tao
            // banditore assegna risorse secondo offerte migliori = invio di un messaggio ai clienti vincitori
            // ai clienti viene detratto il prezzo
            // viene aggiornata la lista delle risorse richieste dal cliente
            // deallocare memoria condivisa
 */

/* Client's bid in the tao */
typedef struct _bid{
    int client_pid;
    int quantity;
    int unit_offer;
} bid;

/**
 * Contiene i bids = offerte dei clienti
 * Manca taoInformation = clienti interessati a quel tao
 */
typedef struct _tao{
    int id;
    char name[MAX_RES_NAME_LENGTH];
    pid_t interested_clients[MAX_CLIENTS];
    int interested_clients_count;
    bid bids[MAX_OFFERS];
    int base_bid; /* base d'asta */
    int shm_id;
    int sem_id;
    int lifetime;
} tao;

/**
 * Support function to make_bid.
 */
void replace_bids(int n, bid* new_bid, tao* auction_tao){
  bid tmp;
  tmp = auction_tao->bids[n];
  tmp.client_pid = new_bid->client_pid;
  tmp.quantity = new_bid->quantity;
  tmp.unit_offer = new_bid->unit_offer;
}


tao** taos;
int taos_count;
int tao_access_semid;

/**
 *  Check if agent in the argument is between the most bids.
 *  @param pid_agent   Agent's pid who wants to make a bid
 *  @param current_tao Tao where he needs to check the bids.
 *  @return 1 if his bid is between best bid, 0 otherwise.
 */
int is_best_bid(int pid_agent, tao* current_tao){
	int i = 0;
	for(; i < MAX_OFFERS; i++){
		if(current_tao->bids[i].client_pid == pid_agent)
			return 1;
	}
	return 0;
}

/**
 * Initializes the TAO array with the given number of required TAOs
 */
void init_taos(int number){
    taos = (tao**) malloc(sizeof(tao) * number);
    taos_count = 0;

    /* Adds a semaphore for each TAO.
     * First semaphore in the pool is reserved to auctioneer (creation max 3 tao at the same time)
     */
    tao_access_semid = semget(IPC_PRIVATE, number, 0666 | IPC_CREAT);
}

/**
 * Initializes the TAO with basic informations.
 * Name is the resource's name.
 */
void create_tao(char name[MAX_RES_NAME_LENGTH]){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    // new_tao->name = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);
    new_tao->id = taos_count;
    strcpy(new_tao->name, name);
    new_tao->interested_clients_count = 0;
    new_tao->base_bid = BASE_BID;
    new_tao->shm_id = -1;
    new_tao->sem_id = tao_access_semid;

    /* Adds the new TAO to the TAOs array */
    taos[taos_count++] = new_tao;
}

	
tao* get_tao(int i){
    if (i < taos_count)
        return taos[i];
    else
        return 0;
}

tao* get_tao_from_resource(char name[MAX_RES_NAME_LENGTH]){
	int i = 0;
	tao* tmp;
	for(; i < taos_count; i++){
		if(strcmp(get_tao(i)->name, name))
			return get_tao(i);
	}
	return tmp;
}


/**
 * Registers the the client with the given pid to the given TAO (identified by
 * its name)
 */
void sign_to_tao(pid_t pid, char name[MAX_RES_NAME_LENGTH]){
    int i = 0;
    for (; i < taos_count; i++){
        if(strcmp(taos[i]->name, name) == 0){
            taos[i]->interested_clients[taos[i]->interested_clients_count++] = pid;
        }
    }
    printf("[auctioneer] Client with pid %d requested partecipation for resource %s\n", pid, name);
}


/**
 * Shared area creation and defines the lifetime.
 */
void start_tao(tao* current_tao){
	
    int shm_id = shmget(IPC_PRIVATE, sizeof(tao), 0600 | IPC_CREAT);
    if(shm_id == -1){
		perror("shmget");
		exit(EXIT_FAILURE);
	}
	
    tao* t;
    t = (tao*) shmat(shm_id, NULL, 0);

    current_tao->shm_id = shm_id;	
    current_tao->lifetime = current_tao->interested_clients_count * 5;
}

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
