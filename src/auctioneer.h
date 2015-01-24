#ifndef AUCTIONEER_H
#define AUCTIONEER_H

#include "auctioneer.c"

int client_msqid_from_pid(int client_pid);
int is_registered(int pid);
void deregister_client(int pid);
void distribute_msqs();
void load_auct_resources();
void create_taos();
int listen_client_status(int client_msqid);
void notify_tao_creation(tao* created_tao);
void notify_tao_start(tao* created_tao);
void notify_tao_end(tao* created_tao);
void notify_auction_result(int client_pid, char* name, int quantity, int unit_bid);
int get_max_bid(tao* current_tao);
void assign_resources(tao* current_tao);
void listen_introductions();
void create_tao_process(int id_tao, int lifetime, int tao_processes_msqid);
void start_auction_system();
void kill_clients();
static void sigint_signal_handler();
void listen_sigint_signal();
int main(int argc, char** argv);

#endif // AUCTIONEER_H
