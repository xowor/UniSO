#ifndef MAIN_H
#define MAIN_H

#include "main.c"


/**
* Creates the auctioneer process.
*/
int create_auctioneer(int msqid);


/**
* Creates a client process.
*/
int create_client(int msqid, int client_num);


/**
* Generates the auctioneer and some clients.
*/
int main(int argc, char** argv);

#endif // MAIN_H
