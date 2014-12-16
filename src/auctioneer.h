#ifndef AUCTIONEER_H
#define AUCTIONEER_H

#include "auctioneer.c"

typedef struct _client client;      /* ?? */


/**
 * Reads from file all the available resources.
 */
void load_resources();


/**
 * Creates (but doesn't start) all the TAOs (one for each resource)
 */
void create_taos();


/**
 * Sends a message to each client, telling them the given TAO was create
 *
 */
void notify_tao_opened(char* name, int shm_id, int sem_id, int base_bid);


/**
 * Listen to all clients introductions. The function stops when has received as
 * many introductions as the number pf clients created (defined in MAX_CLIENTS);
 */
void listen_introductions();


/**
 * Starts the auction system.
 */
void start_auction();


/**
 * Collects all the IPC garbage.
 */
void ipc_gc();

/**
 * Cleans the heap after quitting (heard is a good pratice...).
 */
void gc();

int main(int argc, char** argv);

#endif // AUCTIONEER_H
