#ifndef TAO_H
#define TAO_H

#include "tao.c"

typedef struct _bid bid;
typedef struct _tao tao;

void create_tao(char name[MAX_RES_NAME_LENGTH]);
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao);
void replace_bids(int n, bid* new_bid, tao* auction_tao);

#endif // TAO_H
