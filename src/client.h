#ifndef CLIENT_H
#define CLIENT_H

#include "client.c"


/**
 * Creates the agent.
 */
void start_agent();



/**
 * Loads the resources required by the client from file.
 */
void load_resources();



/**
*  Sends an introduction message to the auctioneer, telling it which resources
*  this client requires.
*/
void send_introduction();



/**
 * Listen to the auction opening message. The function stops until has received
 * an auction opening message.
 */
void listen_auction_creation();


/**
 * Listen to the auction starting message. The function stops until has received
 * an auction starting message.
 */
void listen_auction_start();



/**
 * Collects all the IPC garbage
 */
void ipc_gc();



/**
 * Cleans the heap after quitting (heard is a good pratice...)
 */
void gc();



int main(int argc, char** argv);


#endif // CLIENT_H
