#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include "so_log.h"
#include "config.h"



// TAO = LISTA DI OFFERTE

// un tao per risorsa / ogni tao contiene max 5 offerte


/* Client's bid in the tao */
typedef struct _bid{
    int client_pid;
    int quantity;
    int unit_offer;
} bid;

// area di memoria condivisa
// typedef struct _tao* taoList;
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
 * Initializes the TAO array with the given number of required TAOs
 */
void init_taos(int number){
    taos = (tao**) malloc(sizeof(tao) * number);
    taos_count = 0;

    /* Adds a semaphore for each TAO */
    tao_access_semid = semget(IPC_PRIVATE, number, S_IRUSR | S_IWUSR);
}

/**
 * Initializes the TAO with basic informations.
 * Name is the resource's name.
 */
void create_tao(char name[MAX_RES_NAME_LENGTH]){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    // new_tao->name = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);
    new_tao->id = taos_count++;
    strcpy(new_tao->name, name);
    new_tao->interested_clients_count = 0;
    new_tao->base_bid = BASE_BID;
    new_tao->shm_id = -1;
    new_tao->sem_id = -1;

    /* Adds the new TAO to the TAOs array */
    taos[taos_count] = new_tao;
}


tao* get_tao(int i){
    return taos[i];
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
 * Start (crates the shared memory area) the TAO with the given name.
 */
 /**
 *  definire la durata dell'asta
 *

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
void start_tao(tao* current_tao){
    /* tao creation */
    int shm_id;
    shm_id = shmget(IPC_PRIVATE, sizeof(tao), IPC_CREAT | 0600);
    if(shm_id == -1)
		perror("shmget");
    tao* t;
    t = (tao*) shmat(shm_id, NULL, 0);

    current_tao->shm_id = shm_id;

    /* semaphore creation */
    int sem_id;
    sem_id = (IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
    if(sem_id == -1)
		perror("");
    int ctl = semctl(sem_id, 1, SETVAL, 1);

    current_tao->sem_id = sem_id;
    /* association of sem and shm to interested client with message */
    // informare clienti con un messaggio <id shm del tao, id semaforo, prezzo base d'asta>
}

/**
 * Adds bid from the agent.
 * Precondizione: l'agente ha già controllato se la sua offerta è tra le migliori --> non fa l'offerta con make bid
 * Precondizione: l'agente ha già aumentato la unit_offer rispetto alla offerta precedente
 * Precondizione: l'agente si può permettere (come budget) la unit_offer
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

    // controlla se l'offerta che vuole fare è maggiore del minimo
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
	}
    return -1;
}
