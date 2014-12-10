#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
    char name[MAX_RES_NAME_LENGTH];
    pid_t interested_clients[MAX_CLIENTS];
    int interested_clients_count;
    int base_bid;
    bid bids[MAX_OFFERS];
    int min_price; /* base d'asta */
    //int riempimento = 0;
    // struct _tao* next;
} tao;

/*
tao* create_tao(){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    int j = 0;
    // new_tao -> id = id;
    // new_tao->singleOffer = so;
    // new_tao->bids = {};
    // if ( *nextNode != NULL)
    //     new_tao->next = *nextNode;
    return new_tao;
}
*/

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


/**
 * Initializes the TAO array with the given number of required TAOs
 */
void init_taos(int number){
    taos = (tao**) malloc(sizeof(tao) * number);
    taos_count = 0;
}


void create_tao(char name[MAX_RES_NAME_LENGTH]){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    // new_tao->name = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);
    strcpy(new_tao->name, name);
    new_tao->interested_clients_count = 0;
    new_tao->base_bid = BASE_BID;

    /* Adds the new TAO to the TAOs array */
    taos[taos_count++] = new_tao;
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
void start_tao(char name[MAX_RES_NAME_LENGTH]){
    // int shm_id;
    // /* tao creation */
    // shm_id = shmget(IPC_PRIVATE, sizeof(tao), IPC_CREAT | 0600);
    // if(shm_id == -1)
    //     perror("shmget");
    //     /* tao attach */
    //     tao* t;
    //     t = (tao*) shmat(shm_id, NULL, 0);
    //     /* tao use */
    //     // settare la base d'asta
    //     t->min_price = avail_resources->list->cost;
    //     int sem_id;
    //     /* semaphore creation */
    //     sem_id = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
    //     if(sem_id == -1)
    //         perror("semget");
    //         /* semaphore regulation // CONTROLLARE I PARAMETRI */
    //         int ctl = semctl(sem_id, 1, SETVAL, 1);
    //         /* get interested client to current resource*/
    //         /* association of sem and shm to interested client with message */
    //         /* timer of 3 seconds before the start of auction */
    //         alarm(3);
    //         if(signal(SIGALRM, alarm_handler) == SIG_ERR)
    //             perror("signal (SIG_ERR) error");
}

/**
 * Adds bid from the client.
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

// /**
//  * Creates a node and adds it at the head of the list.
//  * @param id single tao's id
//  * @param so client's bid
//  * @param nextNode Next node of this tao
//  * @return node of taoList
//  */
// tao* node_creation(int id, bid so){
//     tao* new_tao = (tao*) malloc(sizeof(tao));
//     new_tao -> id = id;
//     new_tao->singleOffer = so;
//     if ( *nextNode != NULL)
//         new_tao->next = *nextNode;
//     return new_tao;
// }
