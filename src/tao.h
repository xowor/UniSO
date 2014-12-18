#ifndef TAO_H
#define TAO_H

#include "tao.c"

/* Client's bid in the tao */
typedef struct _bid bid;


/**
* Contiene i bids = offerte dei clienti
* Manca taoInformation = clienti interessati a quel tao
*/
typedef struct _tao tao;

/**
* Support function to make_bid.
*/
void replace_bids(int n, bid* new_bid, tao* auction_tao);

/**
* Initializes the TAO array with the given number of required TAOs
*/
void init_taos(int number);

/**
* Initializes the TAO with basic informations.
* Name is the resource's name.
*/
void create_tao(char name[MAX_RES_NAME_LENGTH]);

/**
 * Gets the tao with the i index.
 */
tao* get_tao(int i);

/**
 * Gets the tao with the given name.
 */
tao* get_tao_by_name(char name[MAX_RES_NAME_LENGTH]);

/**
* Registers the the client with the given pid to the given TAO (identified by
* its name)
*/
void sign_to_tao(pid_t pid, char name[MAX_RES_NAME_LENGTH]);

/**
 * Start (crates the shared memory area) the TAO with the given name.
 */
void start_tao(tao* current_tao);


/**
 * Adds bid from the agent.
 */
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao);

#endif // TAO_H
