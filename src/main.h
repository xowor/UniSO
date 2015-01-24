#ifndef MAIN_H
#define MAIN_H

#include "main.c"


int create_auctioneer(int master_msqid);
int create_client(int master_msqid, int client_num);
int main(int argc, char** argv);

#endif // MAIN_H
