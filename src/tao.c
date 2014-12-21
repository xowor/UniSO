#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include "so_log.h"
#include "config.h"

int tao_counter;
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
 * Initializes the TAO array with the given number of required TAOs
 * @param number Number of tao to be created.
 */
void init_taos_array(int number){
    taos = (tao**) malloc(sizeof(tao) * number);
    taos_count = 0;
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

tao* get_tao_by_name(char name[MAX_RES_NAME_LENGTH]){
	int i = 0;
	tao* tmp;
	for(; i < taos_count; i++){
		if(strcmp(get_tao(i)->name, name))
			return get_tao(i);
	}
	return tmp;
}

tao* get_tao_by_id(int id){
    int i = 0;
    tao* tmp = 0;
    for(; i < taos_count; i++){
        if(get_tao(i)->id == id)
            return get_tao(i);
        }
        return tmp;
}


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
    /* Put the tao into the shm */
    tao* shm;
    /* Attach to shm */
    if (!(shm = (tao*) shmat(current_tao->shm_id, NULL, 0))) {
        perror("shmat");
        exit(1);
    }

    shm = current_tao;

    /* Enable access to the TAO shm */
    semctl(tao_access_semid, current_tao->id, SETVAL, 1);
}


void init_tao(tao* current_tao){
    int shm_id = shmget(IPC_PRIVATE, sizeof(tao) , 0600 | IPC_CREAT);
    if(shm_id == -1){
		perror("shmget");
		exit(EXIT_FAILURE);
	}

    // tao* t;
    // t = (tao*) shmat(shm_id, NULL, 0);

    current_tao->shm_id = shm_id;
    /* The auction must be at least one second long */ 
    current_tao->lifetime = (current_tao->interested_clients_count * AUCTION_LIFETIME_MULTIPLIER) + 1;

    /* Disable access to the TAO shm */
    semctl(tao_access_semid, current_tao->id, SETVAL, 0);
}
