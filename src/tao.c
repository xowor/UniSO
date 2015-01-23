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

/* Client's bid in the tao */
typedef struct _bid{
    int client_pid;
    int quantity;
    int unit_bid;       /* last bid made from this agent_pid */
} bid;

typedef struct _tao{
    int id;
    char name[MAX_RES_NAME_LENGTH];
    pid_t interested_clients[MAX_CLIENTS];
    int interested_clients_count;
    bid bids[MAX_BIDS];
    int base_bid; /* base d'asta */
    int shm_id;
    int sem_id;
    int lifetime;
    int dummy;
} tao;

int sem_id = 0;
tao** taos;
int taos_count;



/**
 * Initializes the TAO array with the given number of required TAOs
 * @param number Number of tao to be created.
 */
void init_taos_array(int number){
    taos = (tao**) malloc(sizeof(tao) * number);
    taos_count = 0;

    sem_id = semget(IPC_PRIVATE, number, S_IRUSR | S_IWUSR);
}

/**
 * Initializes the TAO with basic informations.
 * Name is the resource's name.
 */
void create_tao(char name[MAX_RES_NAME_LENGTH], int cost){
    int shm_id = shmget(IPC_PRIVATE, sizeof(tao), 0600 | IPC_CREAT);
    if(shm_id == -1){
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    tao* new_tao;
    /* Attach to shm */
    if (!(new_tao = (tao*) shmat(shm_id, NULL, 0))) {
        perror("shmat");
        exit(1);
    }

    // tao* new_tao = (tao*) malloc(sizeof(tao));
    // new_tao->name = (char*) malloc(sizeof(char) * MAX_RES_NAME_LENGTH);
    new_tao->id = taos_count;
    strcpy(new_tao->name, name);
    new_tao->interested_clients_count = 0;
    new_tao->base_bid = cost;
    new_tao->shm_id = shm_id;
    new_tao->sem_id = sem_id;

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
    // printf("[auctioneer] Client with pid %d requested partecipation for resource %s\n", pid, name);
}


/**
 * Shared area creation and defines the lifetime.
 */
void start_tao(tao* current_tao){
    /* Put the tao into the shm */
    // tao* shm;
    // /* Attach to shm */
    // if (!(shm = (tao*) shmat(current_tao->shm_id, NULL, 0))) {
    //     perror("shmat");
    //     exit(1);
    // }

    // memcpy(shm, current_tao, sizeof(current_tao));
    // memcpy(shm->name, current_tao->name, /*sizeof(current_tao)*/MAX_RES_NAME_LENGTH);
    //
    // so_log_i('c', shm->lifetime);
    // so_log_i('c', current_tao->lifetime);
    // shm = current_tao;

    /* Enable access to the TAO shm */
    semctl(sem_id, current_tao->id, SETVAL, 1);
}


void init_tao(tao* current_tao){
    // TO_SEE int shm_id = shmget(IPC_PRIVATE, sizeof(tao), 0600 | IPC_CREAT);
    // if(shm_id == -1){
	// 	perror("shmget");
	// 	exit(EXIT_FAILURE);
	// }

    // tao* t;
    // t = (tao*) shmat(shm_id, NULL, 0);

    // current_tao->shm_id = shm_id;
    /* The auction must be at least one second long */

    //Durata tao proporzionale al num di clienti che vi partecipano
    current_tao->lifetime = (current_tao->interested_clients_count * AUCTION_LIFETIME_MULTIPLIER) + 1;

    /* Disable access to the TAO shm until the tao opens*/
    semctl(sem_id, current_tao->id, SETVAL, 0);
}
