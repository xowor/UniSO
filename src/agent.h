#ifndef AGENT_H
#define AGENT_H

#include "agent.c"


int create_agent_process();


/**
* Support function to make_bid.
*/
void replace_bids(int n, bid* new_bid, tao* auction_tao);


#endif // AGENT_H
