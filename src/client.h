#ifndef CLIENT_H
#define CLIENT_H

#include "client.c"

int agent_msqid_from_pid(int agent_pid);
int create_agent_process();
void notify_tao_info(int pid, int availability, int cost, int shmid, int semid, int base_bid, char resource_name[MAX_RES_NAME_LENGTH], int budget);
void create_agent(char* resource_name, int shmid, int semid, int base_bid);
void listen_msqid();
void load_client_resources();
void send_introduction();
void notify_agent_start(int agent_pid);
void notify_client_status(int status);
void remove_agent_from_list(int agent_pid);
void listen_auction_status();
void sigint_signal_handler();
void listen_sigint_signal();
int main(int argc, char** argv);

#endif // CLIENT_H
