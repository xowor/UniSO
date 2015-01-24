#ifndef AGENT_H
#define AGENT_H

#include "agent.c"


void replace_bids(int n, bid* new_bid, tao* auction_tao);
int make_bid(tao* auction_tao);
void print_auction_status(tao* working_tao);
int is_among_the_best_bids();
int worst_bid();
int best_bid();
int get_availability_resources();
void increment_bid();
int make_action(int tao_id);
void listen_tao_start();
void listen_tao_info();
int main(int argc, char** argv);

#endif // AGENT_H
